#include "logEvent.h"
#include "log.h"

namespace sylar {

LogEvent::LogEvent(Logger::ptr logger, LogLevel::Level level, const char *file,
                   int32_t line, uint32_t elapse, uint32_t threadId,
                   uint32_t fiberId, uint64_t time)
    : file_(file), line_(line), elapse_(elapse), threadId_(threadId),
      fiberId_(fiberId), time_(time), logger_(logger), level_(level) {}

auto LogEvent::format(const char *fmt, ...) -> void {
  va_list al;
  va_start(al, fmt);
  format(fmt, al);
  va_end(al);
}

auto LogEvent::format(const char *fmt, va_list al) -> void {
  char *buf = nullptr;
  int len = vasprintf(&buf, fmt, al);
  if (len != -1) {
    ss_ << std::string(buf, len);
    free(buf);
  }
}

LogEventWarp::LogEventWarp(LogEvent::ptr p) : event_(p) {}

LogEventWarp::~LogEventWarp() {
  event_->getLogger()->log(event_->getLogerLevel(), event_);
}

auto LogEventWarp::getSS() -> std::stringstream & { return event_->getSS(); }

auto LogEventWarp::getEvent() -> LogEvent::ptr { return event_; }

} // namespace sylar
