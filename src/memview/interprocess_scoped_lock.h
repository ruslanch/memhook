#ifndef MEMHOOK_SRC_MEMVIEW_INTERPROCESS_SCOPED_LOCK_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_INTERPROCESS_SCOPED_LOCK_H_INCLUDED

#include "common.h"

#include <memhook/scoped_signal.h>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/move/unique_ptr.hpp>

namespace memhook
{

struct InterprocessScopedLock : private noncopyable
{
    ScopedSignalLock signal_lock_;
    ScopedSignal     signal_;

    typedef boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        InterprocessMutexScopedLock;

    boost::movelib::unique_ptr<InterprocessMutexScopedLock> plock_;

    InterprocessScopedLock(boost::interprocess::interprocess_mutex &mutex, bool no_lock);
};

} // namespace

#endif
