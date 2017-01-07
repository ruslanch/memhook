#ifndef MEMHOOK_SRC_MEMVIEW_INTERPROCESS_SCOPED_LOCK_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_INTERPROCESS_SCOPED_LOCK_H_INCLUDED

#include "common.h"

#include <memhook/scoped_signal.h>
#include <memhook/scoped_signal_lock.h>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/move/unique_ptr.hpp>

namespace memhook {
  struct InterprocessScopedLock : private noncopyable {
    InterprocessScopedLock(boost::interprocess::interprocess_mutex &mutex, bool no_lock);

    typedef boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
            InterprocessMutexScopedLock;

    ScopedSignalLock m_signal_lock;
    ScopedSignal     m_signal;
    unique_ptr<InterprocessMutexScopedLock> m_plock;
  };

}  // namespace

#endif
