#include "thread.h"
#include "utils.h"

#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace memhook
{

namespace
{
    inline timespec ToTimespec(boost::chrono::nanoseconds const& ns)
    {
        struct timespec ts;
        ts.tv_sec  = static_cast<long>(boost::chrono::duration_cast<boost::chrono::seconds>(ns).count());
        ts.tv_nsec = static_cast<long>((ns - boost::chrono::duration_cast<boost::chrono::seconds>(ns)).count());
        return ts;
    }
} // ns

#define CHECK_PTHREAD_CALL(call, msg) __extension__ ({           \
        const int _err = (call);                                 \
        if (BOOST_UNLIKELY(_err != 0)) {                         \
            BOOST_THROW_EXCEPTION(boost::system::system_error(   \
                _err, boost::system::system_category(), (msg))); \
        }                                                        \
    })

__thread Thread *Thread::s_current_thread = NULL;

Thread::Thread()
    : thread_()
    , stopflag_()
{}

void Thread::Create(ThreadRunnable *runnable)
{
    runnable_ = runnable;
    CHECK_PTHREAD_CALL(pthread_create(&thread_, NULL, ThreadRoutine, this),
        "Failed to create a thread");
}

void Thread::Join(void **ret)
{
    CHECK_PTHREAD_CALL(pthread_join(thread_, ret),
        "Failed to join the thread");
    stopflag_ = false;
}

void Thread::Stop()
{
    stopflag_ = true;
}

bool Thread::IsStopped()
{
    return stopflag_;
}

void *Thread::ThreadRoutine(void *arg)
{
    s_current_thread = static_cast<Thread *>(arg);

    BOOST_ASSERT(s_current_thread != NULL);
    BOOST_ASSERT(s_current_thread->runnable_ != NULL);

    void *ret = s_current_thread->runnable_->Run();
    s_current_thread = NULL;

    return ret;
}

Thread &Thread::Current()
{
    BOOST_ASSERT(s_current_thread != NULL);
    return *s_current_thread;
}

void Mutex::Initialize()
{
    InitRecursiveMutex();
}

void Mutex::Destroy()
{
    CHECK_PTHREAD_CALL(pthread_mutex_destroy(&mutex_),
        "Failed to destroy the mutex");
}

void Mutex::Lock()
{
    CHECK_PTHREAD_CALL(pthread_mutex_lock(&mutex_),
        "Failed to block on the mutex");
}

void Mutex::Unlock()
{
    CHECK_PTHREAD_CALL(pthread_mutex_unlock(&mutex_),
        "Failed to unlock the mutex");
}

void Mutex::InitRecursiveMutex()
{
    pthread_mutexattr_t mutexattr;
    CHECK_PTHREAD_CALL(pthread_mutexattr_init(&mutexattr),
        "Failed to call pthread_mutexattr_init(...)");

    CHECK_PTHREAD_CALL(pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE),
        "Failed to call pthread_mutexattr_settype(...)");

    CHECK_PTHREAD_CALL(pthread_mutex_init(&mutex_, &mutexattr),
        "Failed to create mutex");

    CHECK_PTHREAD_CALL(pthread_mutexattr_destroy(&mutexattr),
        "Failed to call pthread_mutexattr_destroy(...)");
}

void ConditionVariable::Initialize()
{
    CHECK_PTHREAD_CALL(pthread_cond_init(&condition_, NULL),
        "Failed to create a condition variable");
}

void ConditionVariable::Destroy()
{
    CHECK_PTHREAD_CALL(pthread_cond_destroy(&condition_),
        "Failed to destroy the condition variable");
}

void ConditionVariable::Wait(MutexLock &lock)
{
    Mutex *mutex = lock.GetMutex();
    BOOST_ASSERT(mutex != NULL);
    CHECK_PTHREAD_CALL(pthread_cond_wait(&condition_, mutex->GetPthreadMutex()),
        "Failed to call pthread_cond_timedwait(...)");
}

bool ConditionVariable::TimedWait(MutexLock &lock, const timespec &abstime)
{
    BOOST_ASSERT(lock.GetMutex() != NULL);
    const int ret = pthread_cond_timedwait(&condition_, lock.GetMutex()->GetPthreadMutex(), &abstime);
    if (ret == ETIMEDOUT)
    {
        return false;
    }
    else
    {
        CHECK_PTHREAD_CALL(ret, "Failed to call pthread_cond_timedwait(...)");
    }
    return true;
}

bool ConditionVariable::TimedWait(MutexLock &lock,
        const boost::chrono::time_point<
            boost::chrono::system_clock,
            boost::chrono::nanoseconds
        > &tp)
{
    boost::chrono::nanoseconds d = tp.time_since_epoch();
    timespec ts = ToTimespec(d);
    return TimedWait(lock, ts);
}

void ConditionVariable::Signal()
{
    CHECK_PTHREAD_CALL(pthread_cond_signal(&condition_),
        "Failed to call pthread_cond_signal(...)");
}

void ConditionVariable::Broadcast()
{
    CHECK_PTHREAD_CALL(pthread_cond_broadcast(&condition_),
        "Failed to call pthread_cond_broadcast(...)");
}

} // memhook
