#include "interprocess_scoped_lock.h"

namespace memhook {
  namespace mapped_view_detail {
    extern void InterruptHandler(int, siginfo_t *, void *);
  }  // mapped_view_detail

  InterprocessScopedLock::InterprocessScopedLock(
          boost::interprocess::interprocess_mutex &mutex, bool no_lock)
      : m_signal_lock(SIGINT)
      , m_signal(SIGINT, &mapped_view_detail::InterruptHandler)
      , m_plock() {
    if (!no_lock) {
      m_plock.reset(new InterprocessMutexScopedLock(mutex));
    }
  }
}  // memhook
