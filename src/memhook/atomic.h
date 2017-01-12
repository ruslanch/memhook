#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_H_INCLUDED

#include "common.h"

#if defined(BOOST_CLANG)
#  define MEMHOOK_CLANG_VERSION (__clang_major__*10000 + __clang_minor__*100 + __clang_patchlevel__)
#endif

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#  include "atomic-asm-x86.h"
#elif defined(BOOST_GCC_VERSION) && (BOOST_GCC_VERSION >= 40700)
#  include "atomic-gcc.h"
#elif defined(MEMHOOK_CLANG_VERSION) && (MEMHOOK_CLANG_VERSION >= 30400)
#  include "atomic-gcc.h"
#else
#  error "Not implemented yet!"
#endif

#endif
