#ifndef MEMHOOK_INCLUDE_SCOPED_SIGNAL_H_INCLUDED
#define MEMHOOK_INCLUDE_SCOPED_SIGNAL_H_INCLUDED

#include <memhook/common.h>

#include <boost/noncopyable.hpp>

#include <signal.h>

namespace memhook
{

class ScopedSignalLock : private boost::noncopyable
{
    sigset_t old_mask_;
    bool     blocked_;
public:
    ScopedSignalLock()
            : blocked_(false)
    {
        sigset_t new_mask;
        sigfillset(&new_mask);
        blocked_ = (pthread_sigmask(SIG_SETMASK, &new_mask, &old_mask_) == 0);
    }

    explicit ScopedSignalLock(int signum)
            : blocked_(false)
    {
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigdelset(&new_mask, signum);
        blocked_ = (pthread_sigmask(SIG_SETMASK, &new_mask, &old_mask_) == 0);
    }

    ~ScopedSignalLock()
    {
        if (blocked_)
            pthread_sigmask(SIG_SETMASK, &old_mask_, 0);
    }
};

class ScopedSignal : private boost::noncopyable
{
    struct sigaction old_sa_;
    int  signum_;
    bool handle_;
public:
    ScopedSignal(int signum, void (*callback)(int, siginfo_t *, void *))
            : signum_(signum)
            , handle_(false)
    {
        struct sigaction sa;
        sa.sa_flags     = SA_SIGINFO | SA_RESTART;
        sa.sa_sigaction = callback;
        sigfillset(&sa.sa_mask);
        handle_ = (sigaction(signum, &sa, &old_sa_) == 0);
    }

    ~ScopedSignal()
    {
        if (handle_)
        {
            sigaction(signum_, &old_sa_, NULL);
        }
    }
};

} // namespace

#endif
