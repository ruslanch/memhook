#ifndef MEMHOOK_CONFIG_HPP_INCLUDED
#define MEMHOOK_CONFIG_HPP_INCLUDED

#include "sysconfig.hpp"

#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#define BOOST_CHRONO_VERSION 2
#include <boost/config.hpp>

#if (HAVE_GCC_BUILTIN_RETURN_ADDRESS+0) && (HAVE_GCC_BUILTIN_EXTRACT_RETURN_ADDRESS+0)
#   define MEMHOOK_RETURN_ADDRESS(nr) __builtin_extract_return_addr(__builtin_return_address(nr))
#elif (HAVE_GCC_BUILTIN_RETURN_ADDRESS+0)
#   define MEMHOOK_RETURN_ADDRESS(nr) __builtin_return_address(nr)
#else
#   define MEMHOOK_RETURN_ADDRESS(nr) ((void *)0)
#endif

#define MEMHOOK_SYMBOL_INIT(n) __attribute__((__visibility__("default"), constructor (n)))
#define MEMHOOK_SYMBOL_FINI(n) __attribute__((__visibility__("default"), destructor  (n)))
#define MEMHOOK_API            __attribute__((__visibility__("default"), nothrow))

#if (HAVE_GCC_SYNC_VAL_COMPARE_AND_SWAP+0)
#   define MEMHOOK_CAS(val, cmp, newval) __sync_val_compare_and_swap(val, cmp, newval)
#else
// TODO: use asm implementation
#   error __sync_val_compare_and_swap does not supported
#endif

namespace memhook {
    using namespace MEMHOOK_BOOST_NAMESPACE;
} // memhook

#endif // MEMHOOK_CONFIG_HPP_INCLUDED
