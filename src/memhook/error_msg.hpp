#ifndef MEMHOOK_ERROR_MSG_HPP_INCLUDED
#define MEMHOOK_ERROR_MSG_HPP_INCLUDED

#include <memhook/common.hpp>

namespace memhook {

void error_msg(const char *err_s) BOOST_NOEXCEPT_OR_NOTHROW;
void dlsym_error_msg(const char *err_s);

} // memhook

#endif // MEMHOOK_ERROR_MSG_HPP_INCLUDED
