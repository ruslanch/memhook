#ifndef MEMHOOK_INTERPROCESS_SCOPED_LOCK_HPP_INCLUDED
#define MEMHOOK_INTERPROCESS_SCOPED_LOCK_HPP_INCLUDED

#include "common.hpp"
#include "scoped_signal.hpp"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace memhook {

struct interprocess_scoped_lock : private memhook::noncopyable {
    scoped_signal_block signal_block_;
    scoped_signal       signal_;

    typedef interprocess::scoped_lock<interprocess::interprocess_mutex>
        interprocess_mutex_scoped_lock;
    movelib::unique_ptr<interprocess_mutex_scoped_lock> plock_;

    interprocess_scoped_lock(interprocess::interprocess_mutex &mutex, bool no_lock);
};

} // namespace

#endif // MEMHOOK_INTERPROCESS_SCOPED_LOCK_HPP_INCLUDED
