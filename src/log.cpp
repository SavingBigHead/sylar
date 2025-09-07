#include "log.h"

#include <cctype>
#include <iostream>
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
    std::cout << format_->format(level, event);
  }
}

FileLogAppender::FileLogAppender(const std::string &name) : file_name_(name) {}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= level_) {
    file_stream_ << format_->format(level, event);
  }
}

auto FileLogAppender::reopen() -> bool {
  if (file_stream_) {
    file_stream_.close();
  }
  file_stream_.open(file_name_);
  return !!file_stream_;
}

LogFormatter::LogFormatter(const std::string &pattern) : pattern_(pattern) {}

auto LogFormatter::format(LogLevel::Level level, LogEvent::ptr event)
    -> std::string {
  std::stringstream ss;
  for (auto &i : items_) {
    i.format(ss, level, event);
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
      if (isspace(pattern_[n])) {
        break;
      }

      if (fmt_status == 0) {
        if (pattern_[n] == '(') {
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
    }

    if (fmt_status == 0) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
      }
      str = pattern_.substr(i + 1, n - i - 1);
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n;
    } else if (fmt_status == 1) {
      std::cout << "pattern pares error: " << pattern_ << " - "
                << pattern_.substr(i) << std::endl;
      vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
    } else if (fmt_status == 2) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
      }
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n;
    }
  }

  if (!nstr.empty()) {
    vec.push_back(std::make_tuple(nstr, "", 0));
  }
}

class MessageFromatItem : public LogFormatter::FromatItem {
public:
  auto format(std::ostream &os, LogLevel::Level level, LogEvent::ptr event)
      -> void override {
    os << event->getContent();
  }
};

class LevelFromatItem : public LogFormatter::FromatItem {
public:
  auto format(std::ostream &os, LogLevel::Level level, LogEvent::ptr event)
      -> void override {
    os << LogLevel::ToString(level);
  }
};

} // namespace sylar
