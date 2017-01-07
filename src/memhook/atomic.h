#ifndef MEMHOOK_SRC_MEMHOOK_ATOMIC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ATOMIC_H_INCLUDED

#include "common.h"

#if defined(BOOST_GCC)
#  define MEMHOOK_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(BOOST_CLANG)
#  define MEMHOOK_CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

#if ((MEMHOOK_GCC_VERSION+0) >= 40700) || ((MEMHOOK_CLANG_VERSION+0) >= 30400)
#  include "atomic-gcc.h"
#elif defined(__GNUC__) && (defined(__i386) || defined(__x86_64__))
#  include "atomic-asm-x86.h"
#else
#  error "Not implemented yet!"
#endif

#endif
