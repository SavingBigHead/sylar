#pragma once

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <string>

namespace sylar {

class LogEvent {
 public:
  LogEvent();
  using ptr = std::shared_ptr<LogEvent>;

 private:
  const char* file_ = nullptr;
  int32_t line_ = 0;
  uint32_t elapse_ = 0;
  uint32_t threadId_ = 0;
  uint32_t fiberId_ = 0;
  uint64_t time_;
  std::string content_;
};

class LogLevel {
 public:
  enum Level {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
  };
};

class LogFormatter {
 public:
  using ptr = std::shared_ptr<LogFormatter>;
  std::string format(LogEvent::ptr event);
};

// 日志输出地
class LogAppender {
 public:
  using ptr = std::shared_ptr<LogAppender>;
  virtual ~LogAppender();
  virtual void log(LogLevel::Level level, LogEvent::ptr event);

  auto setFormatter(LogFormatter::ptr val) { format_ = val; }
  auto getFormatter() -> LogFormatter::ptr {return this->format_;}

 protected:
  LogLevel::Level level_;
  LogFormatter::ptr format_;
};


// 日志器
class Logger {
 public:
  using ptr = std::shared_ptr<Logger>;

  Logger(const std::string& name = "root");

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

 private:
  std::string name_;
  LogLevel::Level level_;
  std::list<LogAppender::ptr> appenders_;
};

class StdoutLogAppender : public LogAppender {
 public:
  using ptr = std::shared_ptr<StdoutLogAppender>;
  void log(LogLevel::Level level, LogEvent::ptr event) override;
};

class FileLogAppender : public LogAppender {
 public:
  using ptr = std::shared_ptr<FileLogAppender>;
  FileLogAppender(const std::string& filename);
  void log(LogLevel::Level level, LogEvent::ptr event) override;
  auto reopen() -> bool;

 private:
  std::string file_name_;
  std::ofstream file_stream_;
};

}  // namespace sylar