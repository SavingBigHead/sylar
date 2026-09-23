#include "logAppender.h"
#include "logLevel.h"
#include "yaml-cpp/yaml.h"

#include <iostream>
#include <memory>
#include <sstream>
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

auto StdoutLogAppender::toYamlString() -> std::string {
  YAML::Node node;
  node["type"] = "StdoutLogAppender";
  node["level"] = LogLevel::ToString(level_);
  if (format_) {
    node["formatter"] = format_->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
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

auto FileLogAppender::toYamlString() -> std::string {
  YAML::Node node;
  node["type"] = "FileLogAppender";
  node["file"] = file_name_;
  std::stringstream ss;
  ss << node;
  return ss.str();
}

} // namespace sylar
