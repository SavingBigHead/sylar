#include "log.h"
#include "util.h"

auto main() -> int {
  sylar::Logger::ptr log(new sylar::Logger);
  log->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
  sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0,
                                                 sylar::GetThreadId(),
                                                 sylar::GetFiberId(), time(0)));
  event->getSS() << "hello sylar";

  log->log(sylar::LogLevel::DEBUG, event);

  return 0;
}
