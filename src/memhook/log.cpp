#include "log.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if (HAVE_SYS_SYSCALL_H + 0)
#include <sys/syscall.h>
#endif

namespace memhook {
  void LogWriteToFile(int fileno, const char *buf, size_t len) {
#if (HAVE_SYS_SYSCALL_H+0)
    ::syscall(SYS_write, STDERR_FILENO, buf, len);
#else
    ::write(STDERR_FILENO, buf, len);
#endif
  }

  void LogPrintf(LogSeverity sv, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    LogVPrintf(sv, format, ap);
    va_end(ap);
  }

  void LogVPrintf(LogSeverity sv, const char *format, va_list ap) {
    char buf[512];
    vsnprintf(buf, sizeof(buf) - 1, format, ap);
    size_t buf_len = strlen(buf);
    if (buf_len != 0 && buf[buf_len - 1] != '\n') {
      buf[buf_len] = '\n';
      buf_len += 1;
    }
    LogWriteToFile(STDERR_FILENO, buf, buf_len);
    if (sv == kFATAL) {
      abort();
    }
  }
}
