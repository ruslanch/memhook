#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_GCC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_GCC_H_INCLUDED

namespace memhook {
  typedef int32_t Atomic32;

  inline void MemoryBarrier() {
    __sync_synchronize();
  }

  inline Atomic32 NoBarrier_Load(volatile const Atomic32 *ptr) {
    return *ptr;
  }

  inline void NoBarrier_Store(volatile Atomic32 *ptr, Atomic32 value) {
    *ptr = value;
  }

  inline void Acquire_Store(volatile Atomic32 *ptr, Atomic32 value) {
    *ptr = value;
    MemoryBarrier();
  }

  inline Atomic32 Release_Load(volatile const Atomic32 *ptr) {
    MemoryBarrier();
    return *ptr;
  }

  inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32 *ptr,
          Atomic32 old_value, Atomic32 new_value) {
    Atomic32 prev_value = old_value;
    __atomic_compare_exchange_n(ptr, &prev_value, new_value, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    return prev_value;
  }

  inline Atomic32 Release_AtomicExchange(volatile Atomic32* ptr, Atomic32 new_value) {
    return __atomic_exchange_n(const_cast<Atomic32*>(ptr), new_value, __ATOMIC_RELEASE);
  }
}

#endif
