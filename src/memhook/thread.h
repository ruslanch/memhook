#ifndef MEMHOOK_SRC_MEMHOOK_THREAD_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_THREAD_H_INCLUDED

#include "common.h"
#include "atomic.h"

#include <boost/chrono/ceil.hpp>
#include <boost/move/core.hpp>

#include <pthread.h>

namespace memhook {
  namespace {
    inline timespec ConvToTimeSpec(chrono::nanoseconds const& ns)
    {
      const chrono::seconds s = chrono::duration_cast<chrono::seconds>(ns);
      timespec ts;
      ts.tv_sec  = static_cast<long>(s.count());
      ts.tv_nsec = static_cast<long>((ns - s).count());
      return ts;
    }
  }

  class Mutex : noncopyable {
  public:
    Mutex();
    ~Mutex();

    void Lock();
    void Unlock();

    pthread_mutex_t *GetPthreadMutex() { return &m_mutex; }

  private:
    pthread_mutex_t m_mutex;
  };

  class MutexLock {
  public:
    MutexLock() : m_mutex(NULL), m_is_locked(false) {}
    MutexLock(Mutex &mutex) : m_mutex(&mutex), m_is_locked(false) {
      Lock();
    }

    MutexLock(BOOST_RV_REF(MutexLock) lock)
        : m_mutex(lock.m_mutex), m_is_locked(lock.m_is_locked) {
      lock.m_mutex     = NULL;
      lock.m_is_locked = false;
    }

    MutexLock &operator=(BOOST_RV_REF(MutexLock) lock) {
      Unlock();
      m_mutex     = lock.m_mutex;
      m_is_locked = lock.m_is_locked;
      lock.m_mutex     = NULL;
      lock.m_is_locked = false;
      return *this;
    }

    ~MutexLock() { Unlock(); }

    void Lock() {
      if (!m_is_locked) {
        m_mutex->Lock();
        m_is_locked = true;
      }
    }

    void Unlock() {
      if (m_is_locked) {
        m_mutex->Unlock();
        m_is_locked = false;
      }
    }

    bool IsOwnsLock() const {
      return m_is_locked;
    }

    void swap(MutexLock &lock) {
      std::swap(m_mutex,     lock.m_mutex);
      std::swap(m_is_locked, lock.m_is_locked);
    }

    Mutex *GetMutex() {
        return m_mutex;
    }

  private:
    Mutex *m_mutex;
    bool   m_is_locked;

    BOOST_MOVABLE_BUT_NOT_COPYABLE(MutexLock);
  };

  class ConditionVariable : noncopyable {
  public:

    ConditionVariable();
    ~ConditionVariable();

    void WakeUp();
    void WakeUpAll();

    void Wait(MutexLock &lock);
    bool WaitUntil(MutexLock &lock, const timespec &abs_time);
    bool WaitFor(MutexLock &lock, const timespec &rel_time);

  private:
    pthread_cond_t m_condition;
  };

  class ThreadRunnable {
  public:
    virtual void *Run() = 0;

  protected:
    virtual ~ThreadRunnable() {}
  };

  template <typename T>
  class ThreadRunnableBind0 : public ThreadRunnable {
  public:
    typedef void *(T::*member_t)();

    ThreadRunnableBind0()
        : m_obj()
        , m_mem() {}

    ThreadRunnableBind0(T *obj, member_t mem)
        : m_obj(obj)
        , m_mem(mem) {}

    void Init(T *obj, member_t mem) {
      m_obj = obj;
      m_mem = mem;
    }

    virtual void *Run() {
      BOOST_ASSERT(m_obj != NULL);
      BOOST_ASSERT(m_mem != NULL);
      return (m_obj->*m_mem)();
    }

  private:
    T *m_obj;
    member_t m_mem;
  };

  template <typename T, typename A1>
  class ThreadRunnableBind1 : public ThreadRunnable {
  public:
    typedef void *(T::*member_t)(A1);

    ThreadRunnableBind1()
        : m_obj()
        , m_mem()
        , m_a1() {}

    ThreadRunnableBind1(T *obj, member_t mem,
            typename call_traits<A1>::param_type a1)
        : m_obj(obj)
        , m_mem(mem)
        , m_a1(a1) {}

    void Init(T *obj, member_t mem,
            typename call_traits<A1>::param_type a1) {
      m_obj = obj;
      m_mem = mem;
      m_a1  = a1;
    }

    virtual void *Run() {
      BOOST_ASSERT(m_obj != NULL && m_mem != NULL);
      return (m_obj->*m_mem)(m_a1);
    }

  private:
    T *m_obj;
    member_t m_mem;
    A1 m_a1;
  };

  template <typename T, typename A1, typename A2>
  class ThreadRunnableBind2 : public ThreadRunnable {
  public:
    typedef void *(T::*member_t)(A1, A2);

    ThreadRunnableBind2()
        : m_obj()
        , m_mem()
        , m_a1()
        , m_a2() {}

    ThreadRunnableBind2(T *obj, member_t mem,
            typename call_traits<A1>::param_type a1,
            typename call_traits<A2>::param_type a2)
        : m_obj(obj)
        , m_mem(mem)
        , m_a1(a1)
        , m_a2(a2) {}

    void Init(T *obj, member_t mem,
          typename call_traits<A1>::param_type a1,
          typename call_traits<A2>::param_type a2) {
      m_obj = obj;
      m_mem = mem;
      m_a1 = a1;
      m_a2 = a2;
    }

    virtual void *Run() {
      BOOST_ASSERT(m_obj != NULL && m_mem != NULL);
      return (m_obj->*m_mem)(m_a1, m_a2);
    }

  private:
    T *m_obj;
    member_t m_mem;
    A1 m_a1;
    A2 m_a2;
  };

  class Thread : noncopyable {
  public:
    Thread()
        : m_thread()
        , m_runnable() {}

    virtual ~Thread();

    void Create(ThreadRunnable *runnable);
    void Join(void **ret = NULL);

    static Thread &Current();

  private:
    static void *ThreadRoutine(void *arg);

    pthread_t       m_thread;
    ThreadRunnable *m_runnable;

    static __thread Thread *s_current_thread;
  };

  class InterruptibleThread : public Thread {
  public:
    InterruptibleThread()
        : Thread()
        , m_interrupted()
        , m_sleep_mutex()
        , m_sleep_cond() {}

    void Interrupt();
    bool IsInterruptionRequested() { return (Release_Load(&m_interrupted) != 0); }

    bool SleepUntil(const timespec &abs_time);
    bool SleepFor(const timespec &rel_time);

    template <typename Duration>
    bool SleepUntil(const chrono::time_point<chrono::system_clock, Duration> &abs_time) {
      const chrono::time_point<chrono::system_clock, chrono::nanoseconds> tp(
              chrono::ceil<chrono::nanoseconds>(abs_time.time_since_epoch()));
      return SleepUntil(ConvToTimeSpec(tp.time_since_epoch()));
    }

    template <typename Rep, typename Period>
    bool SleepFor(const chrono::duration<Rep, Period> &rel_time) {
      return SleepFor(ConvToTimeSpec(chrono::duration_cast<chrono::nanoseconds>(rel_time)));
    }

  private:
    Atomic32          m_interrupted;
    Mutex             m_sleep_mutex;
    ConditionVariable m_sleep_cond;
  };
}

#endif
