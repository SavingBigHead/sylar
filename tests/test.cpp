#include "log.h"
#include <iostream>

auto main() -> int {
  sylar::Logger::ptr log(new sylar::Logger);
  log->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
  sylar::LogEvent::ptr event(
      new sylar::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
  event->getSS() << "hello sylar";

  log->log(sylar::LogLevel::DEBUG, event);

  std::cout << "hello" << std::endl;
  return 0;
}
