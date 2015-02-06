#ifndef MEMHOOK_CONFIG_HPP_INCLUDED
#define MEMHOOK_CONFIG_HPP_INCLUDED

#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#define BOOST_CHRONO_VERSION 2
#include <boost/config.hpp>

#define MEMHOOK_CAS(val, cmp, newval) __sync_val_compare_and_swap(val, cmp, newval)
#define MEMHOOK_NOTHROW        __attribute__((nothrow))
#define MEMHOOK_NO_INSTRUMENT  __attribute__((no_instrument_function))
#define MEMHOOK_API            __attribute__((__visibility__("default"), nothrow))

#define MEMHOOK_SYMBOL_INIT(n) \
    __attribute__((__visibility__("default"), constructor (n)))

#define MEMHOOK_SYMBOL_FINI(n) \
    __attribute__((__visibility__("default"), destructor (n)))

#define MEMHOOK_RETURN_ADDRESS(nr) \
    __builtin_extract_return_addr(__builtin_return_address(nr))

namespace memhook {
    using namespace MEMHOOK_BOOST_NAMESPACE;
} // memhook

#endif // MEMHOOK_CONFIG_HPP_INCLUDED
