#ifndef MEMHOOK_INCLUDE_BYTESWAP_H_INCLUDED
#define MEMHOOK_INCLUDE_BYTESWAP_H_INCLUDED

#include <memhook/common.h>

#if (HAVE_GLIBC_BYTESWAP+0)
#   include <byteswap.h>
#   define MEMHOOK_BYTESWAP_16(x) bswap_16(x)
#   define MEMHOOK_BYTESWAP_32(x) bswap_32(x)
#   define MEMHOOK_BYTESWAP_64(x) bswap_64(x)
#elif (HAVE_GLIBC_BITS_BYTESWAP+0)
#   include <byteswap.h>
#   define MEMHOOK_BYTESWAP_16(x) __bswap_16(x)
#   define MEMHOOK_BYTESWAP_32(x) __bswap_32(x)
#   define MEMHOOK_BYTESWAP_64(x) __bswap_64(x)
#elif (HAVE_BSD_BYTESWAP+0)
#   include <sys/endian.h>
#   define MEMHOOK_BYTESWAP_16(x) swap16(x)
#   define MEMHOOK_BYTESWAP_32(x) swap32(x)
#   define MEMHOOK_BYTESWAP_64(x) swap64(x)
#elif (HAVE_BUILTIN_BYTESWAP_32+0) && (HAVE_BUILTIN_BYTESWAP_64+0)
#   if (HAVE_BUILTIN_BYTESWAP_16+0)
#       define MEMHOOK_BYTESWAP_16(x) __builtin_bswap16(x)
#   else
namespace memhook { namespace bswap_detail {
    BOOST_FORCEINLINE BOOST_CONSTEXPR uint16_t builtin_bswap16(uint16_t val)
        BOOST_NOEXCEPT_OR_NOTHROW { return (a << 8) | (a >> 8); }
}} // memhook::bswap_detail
#       define MEMHOOK_BYTESWAP_16(x) memhook::bswap_detail::builtin_bswap16(x)
#   endif
#   define MEMHOOK_BYTESWAP_32(x) __builtin_bswap32(x)
#   define MEMHOOK_BYTESWAP_64(x) __builtin_bswap64(x)
#endif

#if (SYSTEM_BIG_ENDIAN+0)
#   define MEMHOOK_hton_16(x) x
#   define MEMHOOK_ntoh_16(x) x
#   define MEMHOOK_hton_32(x) x
#   define MEMHOOK_ntoh_32(x) x
#   define MEMHOOK_hton_64(x) x
#   define MEMHOOK_ntoh_64(x) x
#else
#   define MEMHOOK_hton_16(x) MEMHOOK_BYTESWAP_16(x)
#   define MEMHOOK_ntoh_16(x) MEMHOOK_BYTESWAP_16(x)
#   define MEMHOOK_hton_32(x) MEMHOOK_BYTESWAP_32(x)
#   define MEMHOOK_ntoh_32(x) MEMHOOK_BYTESWAP_32(x)
#   define MEMHOOK_hton_64(x) MEMHOOK_BYTESWAP_64(x)
#   define MEMHOOK_ntoh_64(x) MEMHOOK_BYTESWAP_64(x)
#endif

namespace memhook
{

#define MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(func, bits) \
    BOOST_FORCEINLINE BOOST_CONSTEXPR uint##bits##_t func##_##bits(uint##bits##_t val) \
        { return MEMHOOK_##func##_##bits(val); }
MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(hton, 16);
MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(ntoh, 16);
MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(hton, 32);
MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(ntoh, 32);
MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(hton, 64);
MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION(ntoh, 64);
#undef MEMHOOK_DEFINE_BYTE_CONVERSION_FUNCTION

} // memhook

#endif
