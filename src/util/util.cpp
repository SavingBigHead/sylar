#include "util.h"
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace sylar {
auto GetThreadId() -> pid_t { return syscall(SYS_gettid); }
auto GetFiberId() -> uint32_t { return 0; }
} // namespace sylar
