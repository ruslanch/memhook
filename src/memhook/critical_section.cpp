#include "critical_section.hpp"
#include "error_msg.hpp"

namespace memhook {

#define MEMHOOK_CHECK_PTHREAD(call) \
    if (BOOST_UNLIKELY((call) != 0)) { error_msg(#call, " failed"); abort(); }


void CriticalSection::init() {
    create_recursive_mutex();
}

void CriticalSection::fini() {
    MEMHOOK_CHECK_PTHREAD(pthread_mutex_destroy(&mutex_));
}

void CriticalSection::lock() {
    MEMHOOK_CHECK_PTHREAD(pthread_mutex_lock(&mutex_));
}

void CriticalSection::unlock() {
    MEMHOOK_CHECK_PTHREAD(pthread_mutex_unlock(&mutex_));
}

void CriticalSection::create_recursive_mutex() {
    pthread_mutexattr_t mutexattr;
    MEMHOOK_CHECK_PTHREAD(pthread_mutexattr_init(&mutexattr));
    MEMHOOK_CHECK_PTHREAD(pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE));
    MEMHOOK_CHECK_PTHREAD(pthread_mutex_init(&mutex_, &mutexattr));
    MEMHOOK_CHECK_PTHREAD(pthread_mutexattr_destroy(&mutexattr));
}

} // ns memhook
