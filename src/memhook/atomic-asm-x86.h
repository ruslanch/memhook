#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_ASM_X86_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_ASM_X86_H_INCLUDED

#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")

namespace memhook {
  struct AtomicOps_x86CPUFeatures {
    bool has_amd_lock_mb_bug;
  };

  extern AtomicOps_x86CPUFeatures g_AtomicOps_x86CPUFeatures;

  typedef int32_t Atomic32;

  inline void MemoryBarrier() {
    __asm__ __volatile__("mfence" : : : "memory");
  }

  inline Atomic32 NoBarrier_Load(const volatile Atomic32 *ptr) {
    return *ptr;
  }

  inline Atomic32 Release_Load(const volatile Atomic32 *ptr) {
    MemoryBarrier();
    return *ptr;
  }

  inline Atomic32 Acquire_Load(const volatile Atomic32 *ptr) {
    Atomic32 value = *ptr;
    COMPILER_BARRIER();
    return value;
  }

  inline void NoBarrier_Store(volatile Atomic32 *ptr, Atomic32 value) {
    *ptr = value;
  }

  inline void Release_Store(volatile Atomic32 *ptr, Atomic32 value) {
    COMPILER_BARRIER();
    *ptr = value;
  }

  inline void Acquire_Store(volatile Atomic32 *ptr, Atomic32 value) {
    *ptr = value;
    MemoryBarrier();
  }

  inline Atomic32 NoBarrier_AtomicExchange(volatile Atomic32* ptr, Atomic32 new_value) {
    __asm__ __volatile__("xchgl %1,%0"
                         : "=r" (new_value)
                         : "m" (*ptr), "0" (new_value)
                         : "memory");
    return new_value;
  }

  inline Atomic32 Release_AtomicExchange(volatile Atomic32* ptr, Atomic32 new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }

  inline Atomic32 Acquire_AtomicExchange(volatile Atomic32* ptr, Atomic32 new_value) {
    Atomic32 prev = NoBarrier_AtomicExchange(ptr, new_value);
    return prev;
  }

  inline Atomic32 NoBarrier_CompareAndSwap(volatile Atomic32* ptr,
          Atomic32 old_value, Atomic32 new_value) {
    Atomic32 prev;
    __asm__ __volatile__("lock; cmpxchgl %1,%2"
                         : "=a" (prev)
                         : "q" (new_value), "m" (*ptr), "0" (old_value)
                         : "memory");
    return prev;
  }

  inline Atomic32 Release_CompareAndSwap(volatile Atomic32 *ptr, Atomic32 old_value, Atomic32 new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32 *ptr, Atomic32 old_value, Atomic32 new_value) {
    Atomic32 prev = NoBarrier_CompareAndSwap(ptr, old_value, new_value);
    if (g_AtomicOps_x86CPUFeatures.has_amd_lock_mb_bug) {
      __asm__ __volatile__("lfence" : : : "memory");
    }
    return prev;
  }

#if defined(__x86_64__)
# define MEMHOOK_HAVE_ATOMIC64
  typedef int64_t Atomic64;

  inline Atomic64 NoBarrier_Load(const volatile Atomic64 *ptr) {
    return *ptr;
  }

  inline Atomic64 Release_Load(const volatile Atomic64 *ptr) {
    MemoryBarrier();
    return *ptr;
  }

  inline Atomic64 Acquire_Load(const volatile Atomic64 *ptr) {
    Atomic64 value = *ptr;
    COMPILER_BARRIER();
    return value;
  }

  inline void NoBarrier_Store(volatile Atomic64 *ptr, Atomic64 value) {
    *ptr = value;
  }

  inline void Release_Store(volatile Atomic64 *ptr, Atomic64 value) {
    COMPILER_BARRIER();
    *ptr = value;
  }

  inline void Acquire_Store(volatile Atomic64 *ptr, Atomic64 value) {
    *ptr = value;
    MemoryBarrier();
  }

  inline Atomic64 NoBarrier_AtomicExchange(volatile Atomic64 *ptr, Atomic64 new_value) {
    __asm__ __volatile__("xchgq %1,%0"
                         : "=r" (new_value)
                         : "m" (*ptr), "0" (new_value)
                         : "memory");
    return new_value;
  }

  inline Atomic64 Release_AtomicExchange(volatile Atomic64 *ptr, Atomic64 new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }

  inline Atomic64 Acquire_AtomicExchange(volatile Atomic64 *ptr, Atomic64 new_value) {
    Atomic64 prev = NoBarrier_AtomicExchange(ptr, new_value);
    return prev;
  }

  inline Atomic64 NoBarrier_CompareAndSwap(volatile Atomic64 *ptr,
          Atomic64 old_value, Atomic64 new_value) {
    Atomic64 prev;
    __asm__ __volatile__("lock; cmpxchgq %1,%2"
                         : "=a" (prev)
                         : "q" (new_value), "m" (*ptr), "0" (old_value)
                         : "memory");
    return prev;
  }

  inline Atomic64 Release_CompareAndSwap(volatile Atomic64 *ptr, Atomic64 old_value, Atomic64 new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline Atomic64 Acquire_CompareAndSwap(volatile Atomic64 *ptr, Atomic64 old_value, Atomic64 new_value) {
    Atomic64 prev = NoBarrier_CompareAndSwap(ptr, old_value, new_value);
    if (g_AtomicOps_x86CPUFeatures.has_amd_lock_mb_bug) {
      __asm__ __volatile__("lfence" : : : "memory");
    }
    return prev;
  }
#endif
}
#endif
