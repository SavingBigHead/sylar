#include "logAppender.h"

#include <iostream>
#include <memory>
#include <string>

namespace sylar {

auto StdoutLogAppender::log(std::shared_ptr<Logger> logger,
                            LogLevel::Level level, LogEvent::ptr event)
    -> void {
  if (level >= level_) {
    std::cout << format_->format(logger, level, event);
  }
}

FileLogAppender::FileLogAppender(const std::string &name) : file_name_(name) {
  reopen();
}

auto FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                          LogEvent::ptr event) -> void {
  if (level >= level_) {
    file_stream_ << format_->format(logger, level, event);
  }
}

auto FileLogAppender::reopen() -> bool {
  if (file_stream_) {
    file_stream_.close();
  }
  file_stream_.open(file_name_);
  return !!file_stream_;
}

} // namespace sylar
