#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_ASM_X86_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_ASM_X86_H_INCLUDED

#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")

namespace memhook {
  typedef int32_t atomic32_t;

  inline void MemoryBarrier() {
    __asm__ __volatile__("mfence" : : : "memory");
  }

  inline atomic32_t NoBarrier_Load(const volatile atomic32_t *ptr) {
    return *ptr;
  }

  inline atomic32_t Release_Load(const volatile atomic32_t *ptr) {
    MemoryBarrier();
    return *ptr;
  }

  inline atomic32_t Acquire_Load(const volatile atomic32_t *ptr) {
    atomic32_t value = *ptr;
    COMPILER_BARRIER();
    return value;
  }

  inline void NoBarrier_Store(volatile atomic32_t *ptr, atomic32_t value) {
    *ptr = value;
  }

  inline void Release_Store(volatile atomic32_t *ptr, atomic32_t value) {
    COMPILER_BARRIER();
    *ptr = value;
  }

  inline void Acquire_Store(volatile atomic32_t *ptr, atomic32_t value) {
    *ptr = value;
    MemoryBarrier();
  }

  inline atomic32_t NoBarrier_AtomicExchange(volatile atomic32_t* ptr, atomic32_t new_value) {
    __asm__ __volatile__("xchgl %1,%0"
                         : "=r" (new_value)
                         : "m" (*ptr), "0" (new_value)
                         : "memory");
    return new_value;
  }

  inline atomic32_t Release_AtomicExchange(volatile atomic32_t* ptr, atomic32_t new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }

  inline atomic32_t Acquire_AtomicExchange(volatile atomic32_t* ptr, atomic32_t new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }

  inline atomic32_t NoBarrier_CompareAndSwap(volatile atomic32_t* ptr,
          atomic32_t old_value, atomic32_t new_value) {
    atomic32_t prev;
    __asm__ __volatile__("lock; cmpxchgl %1,%2"
                         : "=a" (prev)
                         : "q" (new_value), "m" (*ptr), "0" (old_value)
                         : "memory");
    return prev;
  }

  inline atomic32_t Release_CompareAndSwap(volatile atomic32_t *ptr, atomic32_t old_value, atomic32_t new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline atomic32_t Acquire_CompareAndSwap(volatile atomic32_t *ptr, atomic32_t old_value, atomic32_t new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline atomic32_t NoBarrier_AtomicFetchAndAdd(volatile atomic32_t *ptr, atomic32_t value) {
    atomic32_t temp = value;
    __asm__ __volatile__("lock; xaddl %0,%1" : "+r"(temp), "+m"(*ptr) : : "memory");
    return temp;
  }

  inline atomic32_t NoBarrier_AtomicAddAndFetch(volatile atomic32_t *ptr, atomic32_t value) {
    atomic32_t temp = value;
    __asm__ __volatile__("lock; xaddl %0,%1" : "+r"(temp), "+m"(*ptr) : : "memory");
    return temp + value;
  }

  inline atomic32_t Barrier_AtomicFetchAndAdd(volatile atomic32_t *ptr, atomic32_t value) {
    return NoBarrier_AtomicFetchAndAdd(ptr, value);
  }

  inline atomic32_t Barrier_AtomicAddAndFetch(volatile atomic32_t *ptr, atomic32_t value) {
    return NoBarrier_AtomicAddAndFetch(ptr, value);
  }

#ifdef __x86_64__
# define MEMHOOK_HAVE_ATOMIC64
  typedef int64_t atomic64_t;

  inline atomic64_t NoBarrier_Load(const volatile atomic64_t *ptr) {
    return *ptr;
  }

  inline atomic64_t Release_Load(const volatile atomic64_t *ptr) {
    MemoryBarrier();
    return *ptr;
  }

  inline atomic64_t Acquire_Load(const volatile atomic64_t *ptr) {
    atomic64_t value = *ptr;
    COMPILER_BARRIER();
    return value;
  }

  inline void NoBarrier_Store(volatile atomic64_t *ptr, atomic64_t value) {
    *ptr = value;
  }

  inline void Release_Store(volatile atomic64_t *ptr, atomic64_t value) {
    COMPILER_BARRIER();
    *ptr = value;
  }

  inline void Acquire_Store(volatile atomic64_t *ptr, atomic64_t value) {
    *ptr = value;
    MemoryBarrier();
  }

  inline atomic64_t NoBarrier_AtomicExchange(volatile atomic64_t *ptr, atomic64_t new_value) {
    __asm__ __volatile__("xchgq %1,%0"
                         : "=r" (new_value)
                         : "m" (*ptr), "0" (new_value)
                         : "memory");
    return new_value;
  }

  inline atomic64_t Release_AtomicExchange(volatile atomic64_t *ptr, atomic64_t new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }

  inline atomic64_t Acquire_AtomicExchange(volatile atomic64_t *ptr, atomic64_t new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }

  inline atomic64_t NoBarrier_CompareAndSwap(volatile atomic64_t *ptr,
          atomic64_t old_value, atomic64_t new_value) {
    atomic64_t prev;
    __asm__ __volatile__("lock; cmpxchgq %1,%2"
                         : "=a" (prev)
                         : "q" (new_value), "m" (*ptr), "0" (old_value)
                         : "memory");
    return prev;
  }

  inline atomic64_t Release_CompareAndSwap(volatile atomic64_t *ptr, atomic64_t old_value, atomic64_t new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline atomic64_t Acquire_CompareAndSwap(volatile atomic64_t *ptr, atomic64_t old_value, atomic64_t new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline atomic64_t NoBarrier_AtomicFetchAndAdd(volatile atomic64_t *ptr, atomic64_t value) {
    atomic64_t temp = value;
    __asm__ __volatile__("lock; xaddq %0,%1" : "+r"(temp), "+m"(*ptr) : : "memory");
    return temp;
  }

  inline atomic64_t NoBarrier_AtomicAddAndFetch(volatile atomic64_t *ptr, atomic64_t value) {
    atomic64_t temp = value;
    __asm__ __volatile__("lock; xaddq %0,%1" : "+r"(temp), "+m"(*ptr) : : "memory");
    return temp + value;
  }

  inline atomic64_t Barrier_AtomicFetchAndAdd(volatile atomic64_t *ptr, atomic64_t value) {
    return NoBarrier_AtomicFetchAndAdd(ptr, value);
  }

  inline atomic64_t Barrier_AtomicAddAndFetch(volatile atomic64_t *ptr, atomic64_t value) {
    return NoBarrier_AtomicFetchAndAdd(ptr, value);
  }
#endif
}
#endif
