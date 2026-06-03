#include <cstdint>
#include <sched.h>
namespace sylar {
auto GetThreadId() -> pid_t;
auto GetFiberId() -> uint32_t;
} // namespace sylar
