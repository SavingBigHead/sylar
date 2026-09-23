#include "log.h"
#include "config.h"
#include "logAppender.h"
#include "logFormatter.h"
#include "logLevel.h"
#include <bits/types/locale_t.h>
#include <sstream>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>

namespace sylar {

Logger::Logger(const std::string &name) : name_(name), level_(LogLevel::DEBUG) {
  formatter_.reset(new LogFormatter(
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T<%f:%l>%T%m%T%n"));
}

auto Logger::addAppender(LogAppender::ptr appender) -> void {
  if (!appender->getFormatter()) {
    appender->setFormatter(formatter_);
  }
  appenders_.push_back(appender);
}

auto Logger::delAppender(LogAppender::ptr appender) -> void {
  for (auto it = appenders_.begin(); it != appenders_.end(); it++) {
    if (*it == appender) {
      appenders_.erase(it);
      break;
    }
  }
}

auto Logger::clearAppender() -> void { appenders_.clear(); }

auto Logger::setFormatter(LogFormatter::ptr val) -> void { formatter_ = val; }
auto Logger::setFormatter(const std::string &val) -> void {
  LogFormatter::ptr formatter(new LogFormatter(val));
  if (formatter->isError()) {
    return;
  }
  formatter_ = formatter;
}
auto Logger::getFormatter() -> LogFormatter::ptr { return formatter_; }

auto Logger::log(LogLevel::Level level, LogEvent::ptr event) -> void {
  if (level >= level_) {
    auto self = shared_from_this();
    if (!appenders_.empty()) {
      for (auto &i : appenders_) {
        i->log(self, level, event);
      }
    } else if (root_) {
      root_->log(level, event);
    }
  }
}

auto Logger::debug(LogEvent::ptr event) -> void {
  this->log(LogLevel::Level::DEBUG, event);
}

auto Logger::info(LogEvent::ptr event) -> void {
  this->log(LogLevel::Level::INFO, event);
}

auto Logger::warn(LogEvent::ptr event) -> void { log(LogLevel::WARN, event); }

auto Logger::error(LogEvent::ptr event) -> void { log(LogLevel::ERROR, event); }

auto Logger::fatal(LogEvent::ptr event) -> void { log(LogLevel::FATAL, event); }

auto Logger::toYamlString() -> std::string {
  YAML::Node node;
  node["name"] = name_;
  node["level"] = LogLevel::ToString(level_);
  node["formatter"] = formatter_->getPattern();
  for (auto &i : appenders_) {
    node["appenders"].push_back(YAML::Load(i->toYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

struct LogAppenderDefine {
  int type = 0;
  LogLevel::Level level = LogLevel::UNKNOWN;
  std::string formatter;
  std::string file;

  auto operator==(const LogAppenderDefine &oth) const -> bool {
    return type == oth.type && level == oth.level &&
           formatter == oth.formatter && file == oth.file;
  }
};

struct LogDefine {
  std::string name;
  LogLevel::Level level = LogLevel::UNKNOWN;
  std::string formatter;
  std::vector<LogAppenderDefine> appenders;

  auto operator==(const LogDefine &oth) const -> bool {
    return name == oth.name && level == oth.level &&
           formatter == oth.formatter && appenders == oth.appenders;
  }

  auto operator<(const LogDefine &oth) const -> bool { return name < oth.name; }
};

template <> class LexicalCast<std::string, LogDefine> {
public:
  LogDefine operator()(const std::string &v) {
    YAML::Node n = YAML::Load(v);
    LogDefine ld;
    if (!n["name"].IsDefined()) {
      std::cout << "log config error: name is null, " << n << std::endl;
      throw std::logic_error("log config name is null");
    }
    ld.name = n["name"].as<std::string>();
    ld.level = LogLevel::FromString(
        n["level"].IsDefined() ? n["level"].as<std::string>() : "");
    if (n["formatter"].IsDefined()) {
      ld.formatter = n["formatter"].as<std::string>();
    }

    if (n["appenders"].IsDefined()) {
      // std::cout << "==" << ld.name << " = " << n["appenders"].size() <<
      // std::endl;
      for (size_t x = 0; x < n["appenders"].size(); ++x) {
        auto a = n["appenders"][x];
        if (!a["type"].IsDefined()) {
          std::cout << "log config error: appender type is null, " << a
                    << std::endl;
          continue;
        }
        std::string type = a["type"].as<std::string>();
        LogAppenderDefine lad;
        if (type == "FileLogAppender") {
          lad.type = 1;
          if (!a["file"].IsDefined()) {
            std::cout << "log config error: fileappender file is null, " << a
                      << std::endl;
            continue;
          }
          lad.file = a["file"].as<std::string>();
          if (a["formatter"].IsDefined()) {
            lad.formatter = a["formatter"].as<std::string>();
          }
        } else if (type == "StdoutLogAppender") {
          lad.type = 2;
          if (a["formatter"].IsDefined()) {
            lad.formatter = a["formatter"].as<std::string>();
          }
        } else {
          std::cout << "log config error: appender type is invalid, " << a
                    << std::endl;
          continue;
        }

        ld.appenders.push_back(lad);
      }
    }
    return ld;
  }
};

template <> class LexicalCast<LogDefine, std::string> {
public:
  std::string operator()(const LogDefine &i) {
    YAML::Node n;
    n["name"] = i.name;
    if (i.level != LogLevel::UNKNOWN) {
      n["level"] = LogLevel::ToString(i.level);
    }
    if (!i.formatter.empty()) {
      n["formatter"] = i.formatter;
    }

    for (auto &a : i.appenders) {
      YAML::Node na;
      if (a.type == 1) {
        na["type"] = "FileLogAppender";
        na["file"] = a.file;
      } else if (a.type == 2) {
        na["type"] = "StdoutLogAppender";
      }
      if (a.level != LogLevel::UNKNOWN) {
        na["level"] = LogLevel::ToString(a.level);
      }

      if (!a.formatter.empty()) {
        na["formatter"] = a.formatter;
      }

      n["appenders"].push_back(na);
    }
    std::stringstream ss;
    ss << n;
    return ss.str();
  }
};

sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
  LogIniter() {
    g_log_defines->addListener(0xF1E231,
                               [](const std::set<LogDefine> &old_value,
                                  const std::set<LogDefine> &new_value) {
                                 for (auto &i : new_value) {
                                   sylar::Logger::ptr logger;
                                   logger = SYLAR_LOG_BYNAME(i.name);
                                   logger->setLevel(i.level);
                                   if (!i.formatter.empty()) {
                                     logger->setFormatter(i.formatter);
                                   }
                                   logger->clearAppender();
                                   for (auto &a : i.appenders) {
                                     sylar::LogAppender::ptr ap;
                                     if (a.type == 1) {
                                       ap.reset(new FileLogAppender(a.file));
                                     } else if (a.type == 2) {
                                       ap.reset(new StdoutLogAppender());
                                     }
                                     ap->setLevel(a.level);
                                     logger->addAppender(ap);
                                   }
                                 }
                                 // 删除
                                 for (auto &i : old_value) {
                                   auto it = new_value.find(i);
                                   if (it == new_value.end()) {
                                     auto logger = SYLAR_LOG_BYNAME(i.name);
                                     logger->setLevel((LogLevel::Level)100);
                                     logger->clearAppender();
                                   }
                                 }
                               });
  }
};

LoggerManger::LoggerManger() {
  root_.reset(new Logger);
  root_->addAppender(LogAppender::ptr(new StdoutLogAppender));

  loggers_[root_->name_] = root_;
}

auto LoggerManger::toYamlString() -> std::string {
  YAML::Node node;
  for (auto &i : loggers_) {
    node.push_back(YAML::Load(i.second->toYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

auto LoggerManger::getLogger(const std::string &name) -> Logger::ptr {
  auto it = loggers_.find(name);
  if (it == loggers_.end()) {
    Logger::ptr log(new Logger("name"));
    log->root_ = root_;
    loggers_[name] = log;
    return log;
  } else {
    return it->second;
  }
}

} // namespace sylar
