#ifndef MEMHOOK_SRC_MEMHOOK_SPIN_LOCK_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_SPIN_LOCK_H_INCLUDED

#include "common.h"
#include "atomic.h"

namespace memhook {
  class SpinLock : noncopyable {
    enum {
      kSpinLockFree = 0,
      kSpinLockHeld,
      kSpinLockSleeper,
    };
  public:
    enum LinkerInitialized { kLINKER_INITIALIZED };

    SpinLock() : m_lock_value(kSpinLockFree) {}
    explicit SpinLock(LinkerInitialized) {}

    void Lock() {
      if (Acquire_CompareAndSwap(&m_lock_value, kSpinLockFree, kSpinLockHeld) != kSpinLockFree) {
        SlowLock();
      }
    }

    void Unlock() {
      if (Release_AtomicExchange(&m_lock_value, kSpinLockFree) != kSpinLockHeld) {
        SlowUnlock();
      }
    }

  private:
    void SlowLock();
    void SlowUnlock();
    Atomic32 SpinLoop();

    Atomic32 m_lock_value;
  };

  class SpinLockHolder {
  public:
    explicit SpinLockHolder(SpinLock& lock)
        : m_lock(&lock) {
      m_lock->Lock();
    }

    ~SpinLockHolder() {
      m_lock->Unlock();
    }

  private:
    SpinLock* m_lock;
  };
}

#endif
