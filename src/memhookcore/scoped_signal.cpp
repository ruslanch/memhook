#include <memhook/scoped_signal.h>

#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace memhook {
  ScopedSignal::ScopedSignal(int signum, void (*callback)(int, siginfo_t *, void *))
      : m_signum(signum) {
    struct sigaction sa;
    sa.sa_flags     = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = callback;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, m_signum);
    if (BOOST_UNLIKELY(::sigaction(m_signum, &sa, &m_old_sa) != 0)) {
      const int err = errno;
      boost::throw_exception(boost::system::system_error(err,
              boost::system::system_category(),
              "sigaction(...) failed"));
    }
  }

  ScopedSignal::~ScopedSignal() {
    (void)::sigaction(m_signum, &m_old_sa, NULL);
  }
}
