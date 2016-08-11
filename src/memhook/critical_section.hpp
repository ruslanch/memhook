#ifndef MEMHOOK_SRC_MEMHOOK_CRITICAL_SECTION_HPP_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_CRITICAL_SECTION_HPP_INCLUDED

#include "config.hpp"

#include <boost/noncopyable.hpp>
#include <boost/move/move.hpp>
#include <pthread.h>

namespace memhook {

class CriticalSection {
public:
    void init();
    void fini();

    void lock();
    void unlock();

private:
    void create_recursive_mutex();

    pthread_mutex_t mutex_;
};

class CriticalSectionLock {
public:
    CriticalSectionLock()
            : cs_(NULL)
    {}

    CriticalSectionLock(CriticalSection &cs)
            : cs_(&cs) {
        cs_->lock();
    }

    CriticalSectionLock(BOOST_RV_REF(CriticalSectionLock) lock)
            : cs_(lock.cs_) {
        lock.cs_ = NULL;
    }

    CriticalSectionLock &operator=(BOOST_RV_REF(CriticalSectionLock) lock) {
        unlock();

        cs_ = lock.cs_;
        lock.cs_ = NULL;

        return *this;
    }

    ~CriticalSectionLock() {
        unlock();
    }

    void unlock() {
        if (cs_) {
            cs_->unlock();
        }
    }

private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(CriticalSectionLock);

    CriticalSection *cs_;
};

} // ns memhook

#endif
