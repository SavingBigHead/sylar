#include "logLevel.h"

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

auto LogLevel::FromString(const std::string &str) -> LogLevel::Level {
#define XX(level, v)                                                           \
  if (str == #v) {                                                           \
    return LogLevel::level;                                                    \
  }
  XX(DEBUG, debug);
  XX(INFO, info);
  XX(WARN, warn);
  XX(ERROR, error);
  XX(FATAL, fatal);

  XX(DEBUG, DEBUG);
  XX(INFO, INFO);
  XX(WARN, WARN);
  XX(ERROR, ERROR);
  XX(FATAL, FATAL);
  return LogLevel::UNKNOWN;
#undef XX
}

} // namespace sylar
