#pragma once

#include "logAppender.h"
#include "logEvent.h"
#include "logFormatter.h"
#include "logLevel.h"
#include "singleton.h"
#include "util.h"
#include <list>
#include <map>
#include <memory>
#include <string>

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
