#include "log.h"
#include "config.h"
#include "logAppender.h"
#include "logFormatter.h"
#include "logLevel.h"
#include <bits/types/locale_t.h>
#include <string>

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

sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
  LogIniter() {
    g_log_defines->addListener(
        0xF1E231, [](const std::set<LogDefine> &old_value,
                     const std::set<LogDefine> &new_value) {
          for (auto &i : new_value) {
            auto it = old_value.find(i);
            if (it == old_value.end()) {
              // 新增logger

              sylar::Logger::ptr logger(new sylar::Logger(i.name));
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
            } else {
              // 修改
              if (!(i == *it)) {
              }
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
