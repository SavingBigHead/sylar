#pragma once

#include "logEvent.h"
#include "logFormatter.h"
#include <fstream>
#include <memory>
#include <string>

namespace sylar {
class Logger;
// 日志输出地
class LogAppender {
public:
  using ptr = std::shared_ptr<LogAppender>;
  virtual ~LogAppender() {};
  virtual auto log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) -> void = 0;

  auto setFormatter(LogFormatter::ptr val) { format_ = val; }
  auto getFormatter() -> LogFormatter::ptr { return this->format_; }

  auto getLevel() -> LogLevel::Level { return level_; }
  auto setLevel(LogLevel::Level level) -> void { level_ = level; }

  virtual auto toYamlString() -> std::string = 0;

protected:
  LogLevel::Level level_ = LogLevel::DEBUG;
  LogFormatter::ptr format_;
};

class StdoutLogAppender : public LogAppender {
public:
  auto toYamlString() -> std::string override;
  using ptr = std::shared_ptr<StdoutLogAppender>;
  auto log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) -> void override;
};

class FileLogAppender : public LogAppender {
public:
  auto toYamlString() -> std::string override;
  using ptr = std::shared_ptr<FileLogAppender>;
  FileLogAppender(const std::string &filename);
  auto log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) -> void override;
  auto reopen() -> bool;

private:
  std::string file_name_;
  std::ofstream file_stream_;
};

} // namespace sylar
