#include "log.h"
#include "util.h"

auto main() -> int {
  sylar::Logger::ptr log(new sylar::Logger);
  log->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
  // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0,
  //                                               sylar::GetThreadId(),
  //                                               sylar::GetFiberId(),
  //                                               time(0)));
  // event->getSS() << "hello sylar";

  // log->log(sylar::LogLevel::DEBUG, event);

  SYLAR_LOG_INFO(log) << "test macro";
  SYLAR_LOG_ERROR(log) << "test macro error";

  return 0;
}
