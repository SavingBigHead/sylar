#include "log.h"

#include <iostream>

namespace sylar {

Logger::Logger(const std::string &name) : name_(name) {}

void Logger::addAppender(LogAppender::ptr appender) {
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
    for (auto &i : appenders_) {
      i->log(level, event);
    }
  }
}
void Logger::debug(LogEvent::ptr event) {
  this->log(LogLevel::Level::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
  this->log(LogLevel::Level::INFO, event);
}
void Logger::warn(LogEvent::ptr) {}
void Logger::error(LogEvent::ptr) {}
void Logger::fatal(LogEvent::ptr) {}

void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= level_) {
    std::cout << format_->format(event);
  }
}

FileLogAppender::FileLogAppender(const std::string &name) : file_name_(name) {}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= level_) {
    file_stream_ << format_->format(event);
  }
}

auto FileLogAppender::reopen() -> bool {
  if (file_stream_) {
    file_stream_.close();
  }
  file_stream_.open(file_name_);
  return !!file_stream_;
}

}  // namespace sylar