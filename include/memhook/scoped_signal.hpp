#ifndef MEMHOOK_SCOPED_SIGNAL_HPP_INCLUDED
#define MEMHOOK_SCOPED_SIGNAL_HPP_INCLUDED

#include "config.hpp"
#include <boost/noncopyable.hpp>
#include <signal.h>

namespace memhook {

class scoped_signal_block : private boost::noncopyable {
    sigset_t old_mask_;
    bool     blocked_;
public:
    scoped_signal_block()
            : blocked_(false) {
        sigset_t new_mask;
        sigfillset(&new_mask);
        blocked_ = (pthread_sigmask(SIG_SETMASK, &new_mask, &old_mask_) == 0);
    }

    explicit scoped_signal_block(int signum)
            : blocked_(false) {
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigdelset(&new_mask, signum);
        blocked_ = (pthread_sigmask(SIG_SETMASK, &new_mask, &old_mask_) == 0);
    }

    ~scoped_signal_block() {
        if (blocked_)
            pthread_sigmask(SIG_SETMASK, &old_mask_, 0);
    }
};

class scoped_signal : private boost::noncopyable {
    struct sigaction old_sa_;
    int  signum_;
    bool handle_;
public:
    scoped_signal(int signum, void (*callback)(int, siginfo_t *, void *))
        : signum_(signum)
        , handle_(false) {
        struct sigaction sa;
        sa.sa_flags     = SA_SIGINFO | SA_RESTART;
        sa.sa_sigaction = callback;
        sigfillset(&sa.sa_mask);
        handle_ = (sigaction(signum, &sa, &old_sa_) == 0);
    }

    ~scoped_signal() {
        if (handle_)
            sigaction(signum_, &old_sa_, NULL);
    }
};

} // namespace

#endif // MEMHOOK_SCOPED_SIGNAL_HPP_INCLUDED
