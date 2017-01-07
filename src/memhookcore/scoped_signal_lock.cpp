#include <memhook/scoped_signal_lock.h>

#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace memhook {
  namespace {
    sigset_t FillNewMask() {
      sigset_t new_mask;
      sigemptyset(&new_mask);
      sigaddset(&new_mask, SIGINT);
      sigaddset(&new_mask, SIGQUIT);
      sigaddset(&new_mask, SIGABRT);
      sigaddset(&new_mask, SIGTERM);
      return new_mask;
    }

    void SigMask(int how, const sigset_t *set, sigset_t *oldset) {
      const int err = ::pthread_sigmask(how, set, oldset);
      if (BOOST_UNLIKELY(err != 0)) {
        boost::throw_exception(boost::system::system_error(err,
                boost::system::system_category(),
                "pthread_sigmask(...) failed"));
      }
    }

    static const sigset_t g_new_mask = FillNewMask();
  }

  ScopedSignalLock::ScopedSignalLock() {
    SigMask(SIG_SETMASK, &g_new_mask, &m_old_mask);
  }

  ScopedSignalLock::ScopedSignalLock(int signum) {
    sigset_t new_mask = g_new_mask;
    sigdelset(&new_mask, signum);
    SigMask(SIG_SETMASK, &new_mask, &m_old_mask);
  }

  ScopedSignalLock::ScopedSignalLock(int signum1, int signum2) {
    sigset_t new_mask = g_new_mask;
    sigdelset(&new_mask, signum1);
    sigdelset(&new_mask, signum2);
    SigMask(SIG_SETMASK, &new_mask, &m_old_mask);
  }

  ScopedSignalLock::~ScopedSignalLock() {
    (void)::pthread_sigmask(SIG_SETMASK, &m_old_mask, NULL);
  }

} // memhook
