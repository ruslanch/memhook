#include "thread.h"
#include "check.h"

#define CHECK_SUCCEEDED(_call)  MEMHOOK_CHECK((_call) == 0)
#define EXPECT_SUCCEEDED(_call) MEMHOOK_EXPECT((_call) == 0)

namespace memhook {
  namespace {
    timespec operator+=(const timespec &lhs, const timespec &rhs) {
      timespec res = { lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec };
      if (res.tv_nsec >= 1e9l) {
        res.tv_nsec -= 1e9l;
        res.tv_sec  += 1;
      }
      return res;
    }
  }

  Mutex::Mutex()
      : m_mutex() {
    InitRecursiveMutex(&m_mutex);
  }

  Mutex::~Mutex() {
    BOOST_VERIFY(CHECK_SUCCEEDED(pthread_mutex_destroy(&m_mutex)));
  }

  void Mutex::InitRecursiveMutex(pthread_mutex_t *mutex)
  {
    pthread_mutexattr_t mutexattr;
    EXPECT_SUCCEEDED(pthread_mutexattr_init(&mutexattr));
    EXPECT_SUCCEEDED(pthread_mutexattr_settype(&mutexattr,
            PTHREAD_MUTEX_RECURSIVE));
    EXPECT_SUCCEEDED(pthread_mutex_init(mutex, &mutexattr));
    EXPECT_SUCCEEDED(pthread_mutexattr_destroy(&mutexattr));
  }

  void Mutex::Lock() {
    EXPECT_SUCCEEDED(pthread_mutex_lock(&m_mutex));
  }

  void Mutex::Unlock() {
    EXPECT_SUCCEEDED(pthread_mutex_unlock(&m_mutex));
  }

  ConditionVariable::ConditionVariable() {
    EXPECT_SUCCEEDED(pthread_cond_init(&m_condition, NULL));
  }

  ConditionVariable::~ConditionVariable() {
    BOOST_VERIFY(CHECK_SUCCEEDED(pthread_cond_destroy(&m_condition)));
  }

  void ConditionVariable::WakeUp() {
    EXPECT_SUCCEEDED(pthread_cond_signal(&m_condition));
  }

  void ConditionVariable::WakeUpAll() {
    EXPECT_SUCCEEDED(pthread_cond_broadcast(&m_condition));
  }

  void ConditionVariable::Wait(MutexLock &lock) {
    Mutex *mutex = lock.GetMutex();
    BOOST_ASSERT(mutex != NULL);
    EXPECT_SUCCEEDED(pthread_cond_wait(&m_condition, mutex->GetPthreadMutex()));
  }

  bool ConditionVariable::WaitUntil(MutexLock &lock, const timespec &abs_time) {
    BOOST_ASSERT(lock.IsOwnsLock());
    Mutex *mutex = lock.GetMutex();
    const int res = pthread_cond_timedwait(&m_condition, mutex->GetPthreadMutex(), &abs_time);
    if (res == ETIMEDOUT)
      return false;
    EXPECT_SUCCEEDED(res);
    return true;
  }

  bool ConditionVariable::WaitFor(MutexLock &lock, const timespec &rel_time) {
    timespec abs_time;
    EXPECT_SUCCEEDED(clock_gettime(CLOCK_REALTIME, &abs_time));
    abs_time += rel_time;
    return WaitUntil(lock, abs_time);
  }

  __thread Thread *Thread::s_current_thread = NULL;

  Thread::~Thread() {
    BOOST_ASSERT(m_thread == 0);
  }

  void Thread::Create(ThreadRunnable *runnable) {
    BOOST_ASSERT(runnable != NULL);
    m_runnable = runnable;
    EXPECT_SUCCEEDED(pthread_create(&m_thread, NULL, ThreadRoutine, this));
  }

  void Thread::Join(void **ret) {
    EXPECT_SUCCEEDED(pthread_join(m_thread, ret));
    m_thread = 0;
  }

  void *Thread::ThreadRoutine(void *arg) {
    s_current_thread = static_cast<Thread *>(arg);

    BOOST_ASSERT(s_current_thread != NULL);
    BOOST_ASSERT(s_current_thread->m_runnable != NULL);

    void *res = s_current_thread->m_runnable->Run();

    s_current_thread = NULL;
    return res;
  }

  Thread &Thread::Current() {
      BOOST_ASSERT(s_current_thread != NULL);
      return *s_current_thread;
  }

  void InterruptibleThread::Interrupt() {
    Acquire_Store(&m_interrupted, 1);
    MutexLock lock(m_sleep_mutex);
    m_sleep_cond.WakeUpAll();
  }

  bool InterruptibleThread::SleepUntil(const timespec &abs_time) {
    MutexLock lock(m_sleep_mutex);
    while (!IsInterruptionRequested()) {
      if (!m_sleep_cond.WaitUntil(lock, abs_time))
        return true;
    }
    return false;
  }

  bool InterruptibleThread::SleepFor(const timespec &rel_time) {
    MutexLock lock(m_sleep_mutex);
    while (!IsInterruptionRequested()) {
      if (!m_sleep_cond.WaitFor(lock, rel_time))
        return true;
    }
    return false;
  }
}
