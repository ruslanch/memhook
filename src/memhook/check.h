#ifndef MEMHOOK_SRC_MEMHOOK_CHECK_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_CHECK_H_INCLUDED

#include "common.h"
#include "log.h"

#define MEMHOOK_CHECK(_expr) __extension__ ({ \
    const bool _status = ((_expr));           \
    if (BOOST_UNLIKELY(!_status)) {           \
      memhook::LogPrintf(kERROR,              \
              #_expr " failed at %s:%d: "     \
              "\n", __FILE__, __LINE__);      \
    }                                         \
    _status;                                  \
  })

#define MEMHOOK_EXPECT(_expr) __extension__ ({ \
    const bool _status = ((_expr));            \
    if (BOOST_UNLIKELY(!_status)) {            \
      memhook::LogPrintf(kFATAL,               \
              #_expr " failed at %s:%d: "      \
              "\n", __FILE__, __LINE__);       \
    }                                          \
    _status;                                   \
  })

#endif
