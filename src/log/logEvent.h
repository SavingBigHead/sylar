#pragma once

#include "logLevel.h"
#include <cstdint>
#include <memory>
#include <string>

#include <cstdarg>
#include <memory>
#include <sstream>
#include <string>

namespace sylar {

class Logger;

class LogEvent {
public:
  LogEvent() {};
  using ptr = std::shared_ptr<LogEvent>;

  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           const char *file, int32_t line, uint32_t elapse, uint32_t threadId,
           uint32_t fiberId, uint64_t time);

  auto getFile() -> const char * { return file_; }
  auto getLine() -> int32_t { return line_; }
  auto getElapse() -> uint32_t { return elapse_; }
  auto getThreadId() -> uint32_t { return threadId_; }
  auto getFiberId() -> uint32_t { return fiberId_; }
  auto getTime() -> uint64_t { return time_; }
  auto getContent() -> std::string { return ss_.str(); }
  auto getSS() -> std::stringstream & { return ss_; }

  auto getLogger() const -> std::shared_ptr<Logger> { return logger_; }
  auto getLogerLevel() const -> LogLevel::Level { return level_; }

  auto format(const char *fmt, ...) -> void;
  auto format(const char *fmt, va_list al) -> void;

private:
  const char *file_ = nullptr;
  int32_t line_ = 0;
  uint32_t elapse_ = 0;
  uint32_t threadId_ = 0;
  uint32_t fiberId_ = 0;
  uint64_t time_;
  std::stringstream ss_;
  std::shared_ptr<Logger> logger_;
  LogLevel::Level level_;
};

class LogEventWarp {
public:
  LogEventWarp(LogEvent::ptr);
  ~LogEventWarp();
  auto getSS() -> std::stringstream &;
  auto getEvent() -> LogEvent::ptr;

private:
  LogEvent::ptr event_;
};
} // namespace sylar
