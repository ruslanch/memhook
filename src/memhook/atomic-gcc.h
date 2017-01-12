#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_GCC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_GCC_H_INCLUDED

namespace memhook {
  typedef int32_t Atomic32;

  inline void MemoryBarrier() {
    __sync_synchronize();
  }

  inline void NoBarrier_Store(volatile Atomic32 *ptr, Atomic32 value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELAXED);
  }

  inline void Release_Store(volatile Atomic32 *ptr, Atomic32 value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELEASE);
  }

  inline void Acquire_Store(volatile Atomic32 *ptr, Atomic32 value) {
     __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
  }

  inline Atomic32 NoBarrier_Load(const volatile Atomic32 *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
  }

  inline Atomic32 Release_Load(const volatile Atomic32 *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
  }

  inline Atomic32 Acquire_Load(const volatile Atomic32 *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
  }

  inline Atomic32 NoBarrier_AtomicExchange(volatile Atomic32 *ptr, Atomic32 new_value) {
    return __atomic_exchange_n(ptr, new_value, __ATOMIC_RELAXED);
  }

  inline Atomic32 Acquire_AtomicExchange(volatile Atomic32 *ptr, Atomic32 new_value) {
    return __atomic_exchange_n(const_cast<Atomic32 *>(ptr), new_value,  __ATOMIC_ACQUIRE);
  }

  inline Atomic32 Release_AtomicExchange(volatile Atomic32 *ptr, Atomic32 new_value) {
    return __atomic_exchange_n(const_cast<Atomic32 *>(ptr), new_value, __ATOMIC_RELEASE);
  }

  inline Atomic32 NoBarrier_CompareAndSwap(volatile Atomic32 *ptr, Atomic32 old_value, Atomic32 new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    return old_value;
  }

  inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32 *ptr, Atomic32 old_value, Atomic32 new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE);
    return old_value;
  }

  inline Atomic32 Release_CompareAndSwap(volatile Atomic32 *ptr, Atomic32 old_value, Atomic32 new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    return old_value;
  }

#ifdef __LP64__
# define MEMHOOK_HAVE_ATOMIC64
  typedef int64_t Atomic64;

  inline void NoBarrier_Store(volatile Atomic64 *ptr, Atomic64 value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELAXED);
  }

  inline void Release_Store(volatile Atomic64 *ptr, Atomic64 value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELEASE);
  }

  inline void Acquire_Store(volatile Atomic64 *ptr, Atomic64 value) {
     __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
  }

  inline Atomic64 NoBarrier_Load(const volatile Atomic64 *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
  }

  inline Atomic64 Release_Load(const volatile Atomic64 *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
  }

  inline Atomic64 Acquire_Load(const volatile Atomic64 *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
  }

  inline Atomic64 NoBarrier_AtomicExchange(volatile Atomic64 *ptr, Atomic64 new_value) {
    return __atomic_exchange_n(ptr, new_value, __ATOMIC_RELAXED);
  }

  inline Atomic64 Acquire_AtomicExchange(volatile Atomic64 *ptr, Atomic64 new_value) {
    return __atomic_exchange_n(const_cast<Atomic64 *>(ptr), new_value,  __ATOMIC_ACQUIRE);
  }

  inline Atomic64 Release_AtomicExchange(volatile Atomic64 *ptr, Atomic64 new_value) {
    return __atomic_exchange_n(const_cast<Atomic64 *>(ptr), new_value, __ATOMIC_RELEASE);
  }

  inline Atomic64 NoBarrier_CompareAndSwap(volatile Atomic64 *ptr, Atomic64 old_value, Atomic64 new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    return old_value;
  }

  inline Atomic64 Acquire_CompareAndSwap(volatile Atomic64 *ptr, Atomic64 old_value, Atomic64 new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE);
    return old_value;
  }

  inline Atomic64 Release_CompareAndSwap(volatile Atomic64 *ptr, Atomic64 old_value, Atomic64 new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    return old_value;
  }
#endif
}

#endif
