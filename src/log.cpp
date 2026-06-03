#include "log.h"

#include <bits/types/locale_t.h>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace sylar {

auto LogLevel::ToString(LogLevel::Level level) -> const char * {
  switch (level) {
#define XX(name)                                                               \
  case LogLevel::name:                                                         \
    return #name;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
  default:
    return "UNKNOWN";
  }
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
  MessageFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << event->getContent();
  }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
  LevelFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << LogLevel::ToString(level);
  }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
  ElapseFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << event->getElapse();
  }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
  NameFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << logger->getName();
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
  ThreadIdFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << event->getThreadId();
  }
};

class FiberFormatItem : public LogFormatter::FormatItem {
public:
  FiberFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << event->getFiberId();
  }
};

class DataTimeFormatItem : public LogFormatter::FormatItem {
public:
  DataTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
      : format_(format) {
    if (format_.empty()) {
      format_ = "%Y-%m-%d %H:%M:%S";
    }
  };
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    struct tm tm;
    time_t time = event->getTime();
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format_.c_str(), &tm);
    os << buf;
  }

private:
  std::string format_;
};

class FileNameFormatItem : public LogFormatter::FormatItem {
public:
  FileNameFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << event->getFile();
  }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
  LineFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << event->getLine();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
  NewLineFormatItem(const std::string &str = "") {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << std::endl;
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
  StringFormatItem(const std::string &str) : string_(str) {}
  auto format(std::ostream &os, std::shared_ptr<Logger> logger,
              LogLevel::Level level, LogEvent::ptr event) -> void override {
    os << string_;
  }

private:
  std::string string_;
};

LogEvent::LogEvent(const char *file, int32_t line, uint32_t elapse,
                   uint32_t threadId, uint32_t fiberId, uint64_t time)
    : file_(file), line_(line), elapse_(elapse), threadId_(threadId),
      fiberId_(fiberId), time_(time) {}

Logger::Logger(const std::string &name) : name_(name), level_(LogLevel::DEBUG) {
  formatter_.reset(new LogFormatter("%d [%p] <%f:%l> %m %n"));
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

void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                            LogEvent::ptr event) {
  if (level >= level_) {
    std::cout << format_->format(logger, level, event);
  }
}

FileLogAppender::FileLogAppender(const std::string &name) : file_name_(name) {}

void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                          LogEvent::ptr event) {
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

LogFormatter::LogFormatter(const std::string &pattern) : pattern_(pattern) {
  init();
}
auto LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                          LogEvent::ptr event) -> std::string {
  std::stringstream ss;
  for (auto &i : items_) {
    i->format(ss, logger, level, event);
  }
  return ss.str();
}

// %xxx %xxx{xxx} %%
auto LogFormatter::init() -> void {
  // str, format, type
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr;

  for (size_t i{0}; i < pattern_.size(); ++i) {
    if (pattern_[i] != '%') {
      nstr.append(1, pattern_[i]);
      continue;
    }

    if ((i + 1) < pattern_.size()) {
      if (pattern_[i + 1] == '%') {
        nstr.append(1, '%');
        continue;
      }
    }

    size_t n{i + 1};
    int fmt_status{0};
    size_t fmt_begin{0};

    std::string str;
    std::string fmt;

    while (n < pattern_.size()) {
      if (!isalpha(pattern_[n]) && pattern_[n] != '{' && pattern_[n] != '}') {
        break;
      }

      if (fmt_status == 0) {
        if (pattern_[n] == '{') {
          str = pattern_.substr(i + 1, n - i - 1);
          fmt_status = 1;
          fmt_begin = n;
          ++n;
          continue;
        }
      }

      if (fmt_status == 1) {
        if (pattern_[n] == '}') {
          fmt = pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
          fmt_status = 2;
          break;
        }
      }
      ++n;
    }

    if (fmt_status == 0) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
        nstr.clear();
      }
      str = pattern_.substr(i + 1, n - i - 1);
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n - 1;
    } else if (fmt_status == 1) {
      std::cout << "pattern pares error: " << pattern_ << " - "
                << pattern_.substr(i) << std::endl;
      vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
    } else if (fmt_status == 2) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
        nstr.clear();
      }
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n - 1;
    }
  }

  if (!nstr.empty()) {
    vec.push_back(std::make_tuple(nstr, "", 0));
  }

  static std::map<std::string,
                  std::function<FormatItem::ptr(const std::string &str)>>
      s_format_item = {
#define XX(str, C)                                                             \
  {                                                                            \
    #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); }   \
  }
          XX(m, MessageFormatItem),  XX(p, LevelFormatItem),
          XX(r, NameFormatItem),     XX(t, ThreadIdFormatItem),
          XX(n, NewLineFormatItem),  XX(d, DataTimeFormatItem),
          XX(f, FileNameFormatItem), XX(l, LineFormatItem),
#undef XX
      };

  for (auto &i : vec) {
    if (std::get<2>(i) == 0) {
      items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
    } else {
      auto it = s_format_item.find(std::get<0>(i));
      if (it == s_format_item.end()) {
        items_.push_back(FormatItem::ptr(
            new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
      } else {
        items_.push_back(it->second(std::get<1>(i)));
      }
    }
    std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - ("
              << std::get<2>(i) << ")" << std::endl;
  }
}

} // namespace sylar
