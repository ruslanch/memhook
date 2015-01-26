#include "error_msg.hpp"
#include <string.h>

namespace memhook {

/* functions to display messages about errors to stderr */
/* yes, you can use fprintf(stderr, ...), but it is not safe when the program starts */
static const char error_in_dlsym[] = "error in dlsym: ";

void error_msg(const char *err_s) BOOST_NOEXCEPT_OR_NOTHROW {
    const size_t err_s_size = strnlen(err_s, 128);
    const size_t tmpbuf_size = err_s_size + 1;
    char *tmpbuf = (char *)alloca(tmpbuf_size);
    strncpy(tmpbuf, err_s, err_s_size);
    tmpbuf[tmpbuf_size - 1] = '\n';
    write(STDERR_FILENO, tmpbuf, tmpbuf_size);
}

void dlsym_error_msg(const char *err_s) {
    const size_t err_s_size = strnlen(err_s, 128);
    const size_t tmpbuf_size = sizeof(error_in_dlsym) + err_s_size;
    char *tmpbuf = (char *)alloca(tmpbuf_size);
    strncpy(tmpbuf, error_in_dlsym, sizeof(error_in_dlsym));
    strncpy(tmpbuf + sizeof(error_in_dlsym) - 1, err_s, err_s_size);
    tmpbuf[tmpbuf_size - 1] = '\n';
    write(STDERR_FILENO, tmpbuf, tmpbuf_size);
}

} // memhook
