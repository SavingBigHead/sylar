#include "log.h"

auto main() -> int {
  sylar::Logger::ptr log(new sylar::Logger);
  log->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
  // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0,
  //                                               sylar::GetThreadId(),
  //                                               sylar::GetFiberId(),
  //                                               time(0)));
  // event->getSS() << "hello sylar";

  // log->log(sylar::LogLevel::DEBUG, event);

  // sylar::FileLogAppender::ptr file_appender(
  //     new sylar::FileLogAppender("./log.txt"));

  // sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%m%n"));
  // file_appender->setFormatter(fmt);

  // log->addAppender(file_appender);

  SYLAR_LOG_INFO(log) << "test macro";
  SYLAR_LOG_ERROR(log) << "test macro error";

  SYLAR_LOG_FORMAT_ERROR(log, "test fmt %s", "aa");

  auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
  SYLAR_LOG_INFO(log) << "xxx";

  return 0;
}
