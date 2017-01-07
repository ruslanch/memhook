#ifndef MEMHOOK_INCLUDE_SCOPED_SIGNAL_H_INCLUDED
#define MEMHOOK_INCLUDE_SCOPED_SIGNAL_H_INCLUDED

#include <memhook/common.h>
#include <signal.h>

namespace memhook {
  class ScopedSignal : noncopyable {
  public:
    ScopedSignal(int signum, void (*callback)(int, siginfo_t *, void *));
    ~ScopedSignal();

  private:
    struct sigaction m_old_sa;
    int m_signum;
  };
} // namespace

#endif
