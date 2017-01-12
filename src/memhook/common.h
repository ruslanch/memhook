#ifndef MEMHOOK_SRC_MEMHOOK_COMMON_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_COMMON_H_INCLUDED

#include <memhook/common.h>
#include <boost/config.hpp>

#if (HAVE_GCC_BUILTIN_RETURN_ADDRESS+0) && (HAVE_GCC_BUILTIN_EXTRACT_RETURN_ADDRESS+0)
#  define MEMHOOK_RETURN_ADDRESS(nr) __builtin_extract_return_addr(__builtin_return_address(nr))
#elif (HAVE_GCC_BUILTIN_RETURN_ADDRESS+0)
#  define MEMHOOK_RETURN_ADDRESS(nr) __builtin_return_address(nr)
#else
#  define MEMHOOK_RETURN_ADDRESS(nr) ((void *)0)
#endif

#define MEMHOOK_API __attribute__((__visibility__("default")))
#define MEMHOOK_ALIAS(fn) __attribute__((alias(#fn), used))

#ifdef BOOST_NO_CXX11_NOEXCEPT
#  define MEMHOOK_NOEXCEPT
#  define MEMHOOK_USE_NOEXCEPT throw()
#  define MEMHOOK_THROW(_EXC) throw(_EXC)
#else
#  define MEMHOOK_NOEXCEPT noexcept
#  define MEMHOOK_USE_NOEXCEPT noexcept
#  define MEMHOOK_THROW(_EXC)
#endif

#endif
