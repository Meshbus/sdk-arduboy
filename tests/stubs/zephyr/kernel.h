#pragma once
#include <stdint.h>
#include <errno.h>
#define CONFIG_SYS_CLOCK_TICKS_PER_SEC 31250U
namespace kernel_fixture {
inline uint64_t ticks = 0, stop_at = UINT64_MAX;
inline bool finite_exits = true;
inline unsigned waits = 0;
}

#include <mutex>
#include <condition_variable>
struct k_sem { unsigned count; std::mutex mutex; std::condition_variable changed; };
#define K_FOREVER -1
#define K_MSEC(x) (((x) * CONFIG_SYS_CLOCK_TICKS_PER_SEC + 999U) / 1000U)
inline void k_sem_init(k_sem *s,unsigned initial,unsigned) {s->count=initial;}
inline void k_sem_give(k_sem *s) {
    std::lock_guard<std::mutex> guard(s->mutex); ++s->count; s->changed.notify_one();
}
inline int k_sem_take(k_sem *s,int timeout) {
    if (timeout != K_FOREVER) {
        ++kernel_fixture::waits;
        if (kernel_fixture::finite_exits) { return 0; }
        if (kernel_fixture::ticks + timeout >= kernel_fixture::stop_at) {
            kernel_fixture::ticks = kernel_fixture::stop_at; return 0;
        }
        kernel_fixture::ticks += timeout; return -EAGAIN;
    }
    std::unique_lock<std::mutex> guard(s->mutex);
    s->changed.wait(guard, [&] { return s->count != 0; }); --s->count; return 0;
}

#define CONFIG_SYS_CLOCK_TICKS_PER_SEC 31250U
inline uint32_t k_uptime_get_32() { return kernel_fixture::ticks * 1000U / CONFIG_SYS_CLOCK_TICKS_PER_SEC; }
inline int64_t k_uptime_ticks() { return kernel_fixture::ticks; }

#define K_TICKS(x) (x)

inline void *k_sched_current_thread_query() { return nullptr; }
inline int k_thread_stack_space_get(void *, size_t *) { return -1; }
