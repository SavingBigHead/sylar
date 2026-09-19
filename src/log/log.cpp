#include "log.h"
#include <bits/types/locale_t.h>
#include <string>

namespace sylar {

Logger::Logger(const std::string &name) : name_(name), level_(LogLevel::DEBUG) {
  formatter_.reset(new LogFormatter(
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T<%f:%l>%T%m%T%n"));
}

void Logger::addAppender(LogAppender::ptr appender) {
  if (!appender->getFormatter()) {
    appender->setFormatter(formatter_);
  }
  appenders_.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  for (auto it = appenders_.begin(); it != appenders_.end(); it++) {
    if (*it == appender) {
      appenders_.erase(it);
      break;
    }
  }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= level_) {
    auto self = shared_from_this();
    for (auto &i : appenders_) {
      i->log(self, level, event);
    }
  }
}

void Logger::debug(LogEvent::ptr event) {
  this->log(LogLevel::Level::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
  this->log(LogLevel::Level::INFO, event);
}

void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }

void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }

void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

LoggerManger::LoggerManger() {
  root_.reset(new Logger);
  root_->addAppender(LogAppender::ptr(new StdoutLogAppender));
}

auto LoggerManger::getLogger(const std::string &name) -> Logger::ptr {
  auto it = loggers_.find(name);
  if (it == loggers_.end()) {
    return root_;
  } else {
    return it->second;
  }
}

} // namespace sylar
