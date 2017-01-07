#ifndef MEMHOOK_INCLUDE_SCOPED_SIGNAL_LOCK_H_INCLUDED
#define MEMHOOK_INCLUDE_SCOPED_SIGNAL_LOCK_H_INCLUDED

#include <memhook/common.h>
#include <signal.h>

namespace memhook {
  class ScopedSignalLock : noncopyable {
  public:
    ScopedSignalLock();
    explicit ScopedSignalLock(int signum);
    ScopedSignalLock(int signum1, int signum2);
    ~ScopedSignalLock();

  private:
    sigset_t m_old_mask;
  };
}

#endif
