#include "spin_lock.h"
#include "log.h"

#include <unistd.h>
#include <sched.h>
#include <time.h>

#if defined(__GLIBC__) && !defined(_SC_NPROCESSORS_ONLN)
#  include <sys/sysinfo.h>
#endif

#if (HAVE_SYS_SYSCALL_H+0)
#  include <sys/syscall.h>
#endif

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_PRIVATE_FLAG 128

namespace memhook {
  namespace {
    int32_t g_adaptive_spin_count = 0;
    bool    g_have_futex = false;
    int     g_futex_private_flag = FUTEX_PRIVATE_FLAG;

    int SysFutex(int *uaddr, int futex_op, int val, const struct timespec *timeout,
            int *uaddr2, int val3) {
#if (HAVE_SYS_SYSCALL_H+0)
      return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
#else
      return -1;
#endif
    }

    struct SpinLock_InitHelper {
      SpinLock_InitHelper() {
#if defined(_SC_NPROCESSORS_ONLN)
        long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__GLIBC__)
        long cpu_count = get_nprocs();
#else
#  error "Not implemented yet!"
#endif
        if (cpu_count > 1)
          g_adaptive_spin_count = 1000;

        int x = 0;
        g_have_futex = (sizeof(Atomic32) == sizeof(int) &&
                        SysFutex(&x, FUTEX_WAKE, 1, NULL, NULL, 0) >= 0);
        if (g_have_futex && SysFutex(&x, FUTEX_WAKE | g_futex_private_flag, 1, NULL, NULL, 0) < 0)
          g_futex_private_flag = 0;
      }
    } g_init_helper;

    inline void SpinLockPause(void) {
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
      __asm__ __volatile__("rep; nop" : : );
#endif
    }

    int32_t GetSpinLockDelayNS(int loop) {
#ifdef MEMHOOK_HAVE_ATOMIC64
      static Atomic64 rand;
      uint64_t r = static_cast<uint64_t>(NoBarrier_Load(&rand));
      r = 0x5deece66dLL * r + 0xb;
      NoBarrier_Store(&rand, r);
      r <<= 16;
      if (loop < 0 || loop > 32)
        loop = 32;
      return r >> (44 - (loop >> 3));
#else
      static Atomic32 rand;
      uint32_t r = static_cast<uint32_t>(NoBarrier_Load(&rand));
      r = 0x343fd * r + 0x269ec3;
      NoBarrier_Store(&rand, r);
      r <<= 1;
      if (loop < 0 || loop > 32)
        loop = 32;
      return r >> (12 - (loop >> 3));
#endif
    }

    void SpinLockDelay(volatile Atomic32 *ptr, int32_t value, uint32_t loop) {
      int saved_errno = errno;
      if (loop == 0) {
        // do nothing
      } else if (loop == 1) {
#if (HAVE_SCHED_YIELD+0)
        sched_yield();
#elif (HAVE_PTHREAD_YIELD+0)
        pthread_yield();
#else
        struct timespec tm;
        tm.tv_sec = 0;
        tm.tv_nsec = 0;
        nanosleep(&tm, NULL);
#endif
      } else {
        struct timespec tm;
        tm.tv_sec = 0;
        tm.tv_nsec = GetSpinLockDelayNS(loop);
        if (g_have_futex) {
          tm.tv_nsec *= 32;
          SysFutex(reinterpret_cast<int *>(const_cast<Atomic32 *>(ptr)),
                  FUTEX_WAIT | g_futex_private_flag, value, &tm, NULL, 0);
        } else {
          nanosleep(&tm, NULL);
        }
      }
      errno = saved_errno;
    }

    void SpinLockWake(volatile Atomic32 *ptr, bool all) {
      if (g_have_futex) {
        SysFutex(reinterpret_cast<int *>(const_cast<Atomic32 *>(ptr)),
                  FUTEX_WAKE | g_futex_private_flag, all ? INT_MAX : 1,
                  NULL, NULL, 0);
      }
    }
  }

  Atomic32 SpinLock::SpinLoop() {
    int32_t c = g_adaptive_spin_count;
    while (NoBarrier_Load(&m_lock_value) != kSpinLockFree && --c > 0)
      SpinLockPause();
    return Acquire_CompareAndSwap(&m_lock_value, kSpinLockFree, kSpinLockSleeper);
  }

  void SpinLock::SlowLock() {
    Atomic32 lock_value = SpinLoop();
    for (int32_t loop = 0; lock_value != kSpinLockFree;) {
      if (lock_value == kSpinLockHeld) {
        lock_value = Acquire_CompareAndSwap(&m_lock_value, kSpinLockHeld, kSpinLockSleeper);
        if (lock_value == kSpinLockHeld) {
          lock_value = kSpinLockSleeper;
        } else if (lock_value == kSpinLockFree) {
          lock_value = Acquire_CompareAndSwap(&m_lock_value, kSpinLockFree, kSpinLockSleeper);
          continue;
        }
      }
      SpinLockDelay(&m_lock_value, lock_value, ++loop);
      lock_value = SpinLoop();
    }
  }

  void SpinLock::SlowUnlock() {
    SpinLockWake(&m_lock_value, false);
  }
}
