#pragma once

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace sylar {

class Logger;
class LogEvent {
public:
  LogEvent();
  using ptr = std::shared_ptr<LogEvent>;

  LogEvent(const char *file, int32_t line, uint32_t elapse, uint32_t threadId,
           uint32_t fiberId, uint64_t time);

  auto getFile() -> const char * { return file_; }
  auto getLine() -> int32_t { return line_; }
  auto getElapse() -> uint32_t { return elapse_; }
  auto getThreadId() -> uint32_t { return threadId_; }
  auto getFiberId() -> uint32_t { return fiberId_; }
  auto getTime() -> uint64_t { return time_; }
  auto getContent() -> std::string { return ss_.str(); }
  auto getSS() -> std::stringstream & { return ss_; }

private:
  const char *file_ = nullptr;
  int32_t line_ = 0;
  uint32_t elapse_ = 0;
  uint32_t threadId_ = 0;
  uint32_t fiberId_ = 0;
  uint64_t time_;
  std::stringstream ss_;
};

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

} // namespace sylar
