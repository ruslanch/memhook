#include "error_msg.hpp"
#include <cstring>
#include <unistd.h>

namespace memhook {

/* functions to display messages about errors to stderr */
/* yes, you can use fprintf(stderr, ...), but it is not safe when the program starts */
void error_msg(const char *title, const char *msg) {
  const size_t title_len    = strnlen(title, 128);
  const size_t msg_len      = strnlen(msg, 128);
  const size_t tmp_buf_size = title_len + msg_len + 1;
  char *tmp_buf = (char *)alloca(tmp_buf_size);
  memcpy(tmp_buf, title, title_len);
  memcpy(tmp_buf + title_len, msg, msg_len);
  tmp_buf[tmp_buf_size - 1] = '\n';
  const int ret = ::write(STDERR_FILENO, tmp_buf, tmp_buf_size);
  (void)ret;
}

} // memhook
