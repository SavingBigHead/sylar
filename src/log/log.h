#pragma once

#include "singleton.h"
#include <cstdint>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include "util.h"

#define SYLAR_LOG_LEVEL(logger, level)                                         \
  if (logger->getLevel() <= level)                                             \
  sylar::LogEventWarp(                                                         \
      sylar::LogEvent::ptr(new sylar::LogEvent(                                \
          logger, level, __FILE__, __LINE__, 0, sylar::GetThreadId(),          \
          sylar::GetFiberId(), time(0))))                                      \
      .getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FORMAT_LEVEL(logger, level, fmt, ...)                        \
  if (logger->getLevel() <= level)                                             \
  sylar::LogEventWarp(                                                         \
      sylar::LogEvent::ptr(new sylar::LogEvent(                                \
          logger, level, __FILE__, __LINE__, 0, sylar::GetThreadId(),          \
          sylar::GetFiberId(), time(0))))                                      \
      .getEvent()                                                              \
      ->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FORMAT_DEBUG(logger, fmt, ...)                               \
  SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_INFO(logger, fmt, ...)                                \
  SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_WARN(logger, fmt, ...)                                \
  SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_ERROR(logger, fmt, ...)                               \
  SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FORMAT_FATAL(logger, fmt, ...)                               \
  SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT sylar::LoggerMgr::GetInstance()->getRoot()

namespace sylar {

class Logger;

class LogLevel {
public:
  enum Level {
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
  };

  static auto ToString(LogLevel::Level level) -> const char *;
};

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

class LogFormatter {
public:
  using ptr = std::shared_ptr<LogFormatter>;
  LogFormatter(const std::string &pattern);
  auto format(std::shared_ptr<Logger> logger, LogLevel::Level level,
              LogEvent::ptr event) -> std::string;

  auto init() -> void;

public:
  class FormatItem {
  public:
    using ptr = std::shared_ptr<FormatItem>;
    virtual ~FormatItem() {};
    virtual auto format(std::ostream &os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level, LogEvent::ptr event) -> void = 0;
  };

private:
  std::string pattern_;
  std::vector<FormatItem::ptr> items_;
};

// 日志输出地
class LogAppender {
public:
  using ptr = std::shared_ptr<LogAppender>;
  virtual ~LogAppender() {};
  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) = 0;

  auto setFormatter(LogFormatter::ptr val) { format_ = val; }
  auto getFormatter() -> LogFormatter::ptr { return this->format_; }

  auto getLevel() -> LogLevel::Level { return level_; }
  auto setLevel(LogLevel::Level level) -> void { level_ = level; }

protected:
  LogLevel::Level level_ = LogLevel::DEBUG;
  LogFormatter::ptr format_;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
public:
  using ptr = std::shared_ptr<Logger>;

  Logger(const std::string &name = "root");

  void log(LogLevel::Level level, LogEvent::ptr event);

  void debug(LogEvent::ptr);
  void info(LogEvent::ptr);
  void warn(LogEvent::ptr);
  void error(LogEvent::ptr);
  void fatal(LogEvent::ptr);

  void addAppender(LogAppender::ptr);
  void delAppender(LogAppender::ptr);

  LogLevel::Level getLevel() const { return level_; }
  void setLevel(LogLevel::Level val) { level_ = val; }

  auto getName() const -> const std::string & { return name_; }

private:
  std::string name_;
  LogLevel::Level level_;
  std::list<LogAppender::ptr> appenders_;
  LogFormatter::ptr formatter_;
};

class StdoutLogAppender : public LogAppender {
public:
  using ptr = std::shared_ptr<StdoutLogAppender>;
  void log(Logger::ptr logger, LogLevel::Level level,
           LogEvent::ptr event) override;
};

class FileLogAppender : public LogAppender {
public:
  using ptr = std::shared_ptr<FileLogAppender>;
  FileLogAppender(const std::string &filename);
  void log(Logger::ptr logger, LogLevel::Level level,
           LogEvent::ptr event) override;
  auto reopen() -> bool;

private:
  std::string file_name_;
  std::ofstream file_stream_;
};

class LoggerManger {
public:
  LoggerManger();
  auto getLogger(const std::string &name) -> Logger::ptr;
  auto init() -> void;

  auto getRoot() { return root_; }

private:
  std::map<std::string, Logger::ptr> loggers_;
  Logger::ptr root_;
};

using LoggerMgr = sylar::Singleton<LoggerManger>;

} // namespace sylar
