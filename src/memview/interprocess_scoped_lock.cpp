#include "interprocess_scoped_lock.h"

namespace memhook {
namespace mapped_view_detail {
    extern void InterruptHandler(int, siginfo_t *, void *);
} // mapped_view_detail

InterprocessScopedLock::InterprocessScopedLock(boost::interprocess::interprocess_mutex &mutex,
        bool no_lock)
    : signal_lock_(SIGINT)
    , signal_(SIGINT, &mapped_view_detail::InterruptHandler)
    , plock_()
{
    if (!no_lock)
        plock_.reset(new InterprocessMutexScopedLock(mutex));
}

} // memhook
