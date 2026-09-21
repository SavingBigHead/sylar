#pragma once

#include "logEvent.h"
#include "logLevel.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace sylar {

class Logger;
class LogFormatter {
public:
  using ptr = std::shared_ptr<LogFormatter>;
  LogFormatter(const std::string &pattern);
  auto format(std::shared_ptr<Logger> logger, LogLevel::Level level,
              LogEvent::ptr event) -> std::string;

  auto init() -> void;

  auto isError() -> bool { return error_; }

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
  bool error_ = false;
};

} // namespace sylar
