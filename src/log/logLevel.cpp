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

auto LogLevel::FromString(const std::string &level) -> LogLevel::Level {
#define XX(name)                                                               \
  if (level == #name) {                                                        \
    return LogLevel::name;                                                     \
  }
  XX(DEBUG);
  XX(INFO);
  XX(WARN);
  XX(ERROR);
  XX(FATAL);
  return LogLevel::UNKNOWN;
#undef XX
}

} // namespace sylar
