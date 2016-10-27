#ifndef MEMHOOK_SRC_MEMHOOK_THREAD_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_THREAD_H_INCLUDED

#include "common.h"

#include <boost/move/core.hpp>
#include <boost/noncopyable.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/chrono/ceil.hpp>

#include <pthread.h>

namespace memhook
{

class ThreadRunnable
{
public:
    virtual void *Run() = 0;

protected:
    ~ThreadRunnable() {}
};

template <typename T>
class ThreadRunnableBind0 : public ThreadRunnable
{
public:
    typedef void *(T::*member_t)();

    ThreadRunnableBind0()
        : obj_()
        , mem_()
    {}

    ThreadRunnableBind0(T *obj, member_t mem)
        : obj_(obj)
        , mem_(mem)
    {}

    void Init(T *obj, member_t mem)
    {
        obj_ = obj;
        mem_ = mem;
    }

    virtual void *Run()
    {
        BOOST_ASSERT(obj_ != NULL && mem_ != NULL);
        return (obj_->*mem_)();
    }

private:
    T *obj_;
    member_t mem_;
};

template <typename T, typename A1>
class ThreadRunnableBind1 : public ThreadRunnable
{
public:
    typedef void *(T::*member_t)(A1);

    ThreadRunnableBind1()
        : obj_()
        , mem_()
        , a1_()
    {}

    ThreadRunnableBind1(T *obj, member_t mem, A1 a1)
        : obj_(obj)
        , mem_(mem)
        , a1_(a1)
    {}

    void Init(T *obj, member_t mem, A1 a1)
    {
        obj_ = obj;
        mem_ = mem;
        a1_  = a1;
    }

    virtual void *Run()
    {
        BOOST_ASSERT(obj_ != NULL && mem_ != NULL);
        return (obj_->*mem_)(a1_);
    }

private:
    T *obj_;
    member_t mem_;
    A1 a1_;
};

template <typename T, typename A1, typename A2>
class ThreadRunnableBind2 : public ThreadRunnable
{
public:
    typedef void *(T::*member_t)(A1, A2);

    ThreadRunnableBind2()
        : obj_()
        , mem_()
        , a1_()
    {}

    ThreadRunnableBind2(T *obj, member_t mem, A1 a1, A2 a2)
        : obj_(obj)
        , mem_(mem)
        , a1_(a1)
        , a2_(a2)
    {}

    void Init(T *obj, member_t mem, A1 a1, A2 a2)
    {
        obj_ = obj;
        mem_ = mem;
        a1_  = a1;
        a2_  = a2;
    }

    virtual void *Run()
    {
        BOOST_ASSERT(obj_ != NULL && mem_ != NULL);
        return (obj_->*mem_)(a1_, a2_);
    }

private:
    T *obj_;
    member_t mem_;
    A1 a1_;
    A2 a2_;
};

class Thread : noncopyable
{
public:
    Thread();

    void Create(ThreadRunnable *runnable);
    void Join(void **ret);

    void Stop();
    bool IsStopped();

    static Thread &Current();

private:
    static void *ThreadRoutine(void *arg);

    pthread_t          thread_;
    ThreadRunnable    *runnable_;
    boost::atomic_bool stopflag_;

    static __thread Thread *s_current_thread;
};

class Mutex : noncopyable
{
public:
    void Initialize();
    void Destroy();

    void Lock();
    void Unlock();

    pthread_mutex_t *GetPthreadMutex()
    {
        return &mutex_;
    }

private:
    void InitRecursiveMutex();

    pthread_mutex_t mutex_;
};

class MutexLock
{
public:
    MutexLock()
            : mutex_(NULL)
    {}

    MutexLock(Mutex &cs)
            : mutex_(&cs)
    {
        mutex_->Lock();
    }

    MutexLock(BOOST_RV_REF(MutexLock) lock)
            : mutex_(lock.mutex_)
    {
        lock.mutex_ = NULL;
    }

    MutexLock &operator=(BOOST_RV_REF(MutexLock) lock)
    {
        Unlock();

        mutex_ = lock.mutex_;
        lock.mutex_ = NULL;

        return *this;
    }

    ~MutexLock()
    {
        Unlock();
    }

    void Unlock()
    {
        if (mutex_)
        {
            mutex_->Unlock();
            mutex_ = NULL;
        }
    }

    void swap(MutexLock &lock)
    {
        Mutex *tmp_cs = mutex_;
        mutex_ = lock.mutex_;
        lock.mutex_ = tmp_cs;
    }

    Mutex *GetMutex()
    {
        return mutex_;
    }

private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(MutexLock);

    Mutex *mutex_;
};

class ConditionVariable : noncopyable
{
public:
    void Initialize();
    void Destroy();

    void Signal();
    void Broadcast();

    void Wait(MutexLock &lock);
    bool TimedWait(MutexLock &lock, const timespec &abstime);
    bool TimedWait(MutexLock &lock,
            const boost::chrono::time_point<
                boost::chrono::system_clock,
                boost::chrono::nanoseconds
            > &tp);

    template <class Duration>
    bool TimedWait(MutexLock &lock,
            const boost::chrono::time_point<boost::chrono::system_clock, Duration>& tp)
    {
        return TimedWait(lock,
            boost::chrono::time_point<
                boost::chrono::system_clock,
                boost::chrono::nanoseconds
            >(boost::chrono::ceil<boost::chrono::nanoseconds>(tp.time_since_epoch())));
    }

    template <class Rep, class Period>
    bool TimedWait(MutexLock &lock,
            const boost::chrono::duration<Rep, Period>& rel_time)
    {
        return TimedWait(lock, boost::chrono::system_clock::now() + rel_time);
    }

private:
    pthread_cond_t condition_;
};

} // memhook

#endif
