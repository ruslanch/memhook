#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_ASM_X86_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_ASM_X86_H_INCLUDED

namespace memhook {
  typedef int32_t Atomic32;

  inline void MemoryBarrier() {
    __asm__ __volatile__("mfence" ::: "memory");
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

  inline Atomic32 NoBarrier_CompareAndSwap(volatile Atomic32* ptr,
          Atomic32 old_value, Atomic32 new_value) {
    Atomic32 prev;
    __asm__ __volatile__("lock; cmpxchgl %1,%2"
                         : "=a" (prev)
                         : "q" (new_value), "m" (*ptr), "0" (old_value)
                         : "memory");
    return prev;
  }

  inline Atomic32 NoBarrier_AtomicExchange(volatile Atomic32* ptr, Atomic32 new_value) {
    __asm__ __volatile__("xchgl %1,%0"
                         : "=r" (new_value)
                         : "m" (*ptr), "0" (new_value)
                         : "memory");
    return new_value;
  }

  inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32 *ptr,
          Atomic32 old_value, Atomic32 new_value) {
    return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  }

  inline Atomic32 Release_AtomicExchange(volatile Atomic32* ptr, Atomic32 new_value) {
    return NoBarrier_AtomicExchange(ptr, new_value);
  }
}

#endif
