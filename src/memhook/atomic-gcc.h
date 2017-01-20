#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_GCC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_GCC_H_INCLUDED

namespace memhook {
  typedef int32_t atomic32_t;

  inline void MemoryBarrier() {
    __sync_synchronize();
  }

  inline void NoBarrier_Store(volatile atomic32_t *ptr, atomic32_t value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELAXED);
  }

  inline void Release_Store(volatile atomic32_t *ptr, atomic32_t value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELEASE);
  }

  inline void Acquire_Store(volatile atomic32_t *ptr, atomic32_t value) {
     __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
  }

  inline atomic32_t NoBarrier_Load(const volatile atomic32_t *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
  }

  inline atomic32_t Release_Load(const volatile atomic32_t *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
  }

  inline atomic32_t Acquire_Load(const volatile atomic32_t *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
  }

  inline atomic32_t NoBarrier_AtomicExchange(volatile atomic32_t *ptr, atomic32_t new_value) {
    return __atomic_exchange_n(ptr, new_value, __ATOMIC_RELAXED);
  }

  inline atomic32_t Acquire_AtomicExchange(volatile atomic32_t *ptr, atomic32_t new_value) {
    return __atomic_exchange_n(const_cast<atomic32_t *>(ptr), new_value,  __ATOMIC_ACQUIRE);
  }

  inline atomic32_t Release_AtomicExchange(volatile atomic32_t *ptr, atomic32_t new_value) {
    return __atomic_exchange_n(const_cast<atomic32_t *>(ptr), new_value, __ATOMIC_RELEASE);
  }

  inline atomic32_t NoBarrier_CompareAndSwap(volatile atomic32_t *ptr, atomic32_t old_value, atomic32_t new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    return old_value;
  }

  inline atomic32_t Acquire_CompareAndSwap(volatile atomic32_t *ptr, atomic32_t old_value, atomic32_t new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE);
    return old_value;
  }

  inline atomic32_t Release_CompareAndSwap(volatile atomic32_t *ptr, atomic32_t old_value, atomic32_t new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    return old_value;
  }

  inline atomic32_t NoBarrier_AtomicFetchAndAdd(volatile atomic32_t *ptr, atomic32_t value) {
    return __atomic_fetch_add(ptr, value, __ATOMIC_RELAXED);
  }

  inline atomic32_t NoBarrier_AtomicAddAndFetch(volatile atomic32_t *ptr, atomic32_t value) {
    return __atomic_add_fetch(ptr, value, __ATOMIC_RELAXED);
  }

  inline atomic32_t Barrier_AtomicFetchAndAdd(volatile atomic32_t *ptr, atomic32_t value) {
    return __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST);
  }

  inline atomic32_t Barrier_AtomicAddAndFetch(volatile atomic32_t *ptr, atomic32_t value) {
    return __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST);
  }

#ifdef __x86_64__
# define MEMHOOK_HAVE_ATOMIC64
  typedef int64_t atomic64_t;

  inline void NoBarrier_Store(volatile atomic64_t *ptr, atomic64_t value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELAXED);
  }

  inline void Release_Store(volatile atomic64_t *ptr, atomic64_t value) {
     __atomic_store_n(ptr, value, __ATOMIC_RELEASE);
  }

  inline void Acquire_Store(volatile atomic64_t *ptr, atomic64_t value) {
     __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
  }

  inline atomic64_t NoBarrier_Load(const volatile atomic64_t *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
  }

  inline atomic64_t Release_Load(const volatile atomic64_t *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
  }

  inline atomic64_t Acquire_Load(const volatile atomic64_t *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
  }

  inline atomic64_t NoBarrier_AtomicExchange(volatile atomic64_t *ptr, atomic64_t new_value) {
    return __atomic_exchange_n(ptr, new_value, __ATOMIC_RELAXED);
  }

  inline atomic64_t Acquire_AtomicExchange(volatile atomic64_t *ptr, atomic64_t new_value) {
    return __atomic_exchange_n(const_cast<atomic64_t *>(ptr), new_value,  __ATOMIC_ACQUIRE);
  }

  inline atomic64_t Release_AtomicExchange(volatile atomic64_t *ptr, atomic64_t new_value) {
    return __atomic_exchange_n(const_cast<atomic64_t *>(ptr), new_value, __ATOMIC_RELEASE);
  }

  inline atomic64_t NoBarrier_CompareAndSwap(volatile atomic64_t *ptr, atomic64_t old_value, atomic64_t new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    return old_value;
  }

  inline atomic64_t Acquire_CompareAndSwap(volatile atomic64_t *ptr, atomic64_t old_value, atomic64_t new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE);
    return old_value;
  }

  inline atomic64_t Release_CompareAndSwap(volatile atomic64_t *ptr, atomic64_t old_value, atomic64_t new_value) {
    __atomic_compare_exchange_n(ptr, &old_value, new_value, false, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    return old_value;
  }

  inline atomic64_t NoBarrier_AtomicFetchAndAdd(volatile atomic64_t *ptr, atomic64_t value) {
    return __atomic_fetch_add(ptr, value, __ATOMIC_RELAXED);
  }

  inline atomic64_t NoBarrier_AtomicAddAndFetch(volatile atomic64_t *ptr, atomic64_t value) {
    return __atomic_add_fetch(ptr, value, __ATOMIC_RELAXED);
  }

  inline atomic64_t Barrier_AtomicFetchAndAdd(volatile atomic64_t *ptr, atomic64_t value) {
    return __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST);
  }

  inline atomic64_t Barrier_AtomicAddAndFetch(volatile atomic64_t *ptr, atomic64_t value) {
    return __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST);
  }
#endif
}

#endif
