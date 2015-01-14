#include "basic_mapped_view.hpp"
#include <cxxabi.h>

namespace memhook {
namespace {
    void unique_char_buf_null_free(const char *)    BOOST_NOEXCEPT_OR_NOTHROW {}
    void unique_char_buf_full_free(const char *mem) BOOST_NOEXCEPT_OR_NOTHROW {
        free(const_cast<char *>(mem));
    }
} // namespace

basic_mapped_view_base::unique_char_buf_t basic_mapped_view_base::cxa_demangle(const char *source)
        BOOST_NOEXCEPT_OR_NOTHROW
{
    int ret = 0;
    unique_char_buf_t res(abi::__cxa_demangle(source, NULL, NULL, &ret), unique_char_buf_full_free);
    if (ret == 0 && res)
        return move(res);
    return unique_char_buf_t(source, unique_char_buf_null_free);
}

} // memhook
