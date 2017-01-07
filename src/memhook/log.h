#ifndef MEMHOOK_SRC_MEMHOOK_LOG_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_LOG_H_INCLUDED

#include "common.h"
#include <stdarg.h>

namespace memhook {
  enum LogSeverity {
    kINFO    = -1,
    kWARNING = -2,
    kERROR   = -3,
    kFATAL   = -4
  };

  void LogWriteToFile(int fileno, const char *buf, size_t len);
  void LogPrintf(LogSeverity sv, const char *format, ...)
          __attribute__((format(printf, 2, 3)));
  void LogVPrintf(LogSeverity sv, const char *format, va_list ap)
          __attribute__((format(printf, 2, 0)));
}

#endif
