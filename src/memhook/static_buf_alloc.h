#ifndef MEMHOOK_SRC_MEMHOOK_STATIC_BUF_ALLOC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_STATIC_BUF_ALLOC_H_INCLUDED

#include "common.h"

#include <cstddef>

namespace memhook {
  /* simple allocator used when the program starts */
  class StaticBufAlloc {
  public:
    static void* malloc(size_t size);
    static void* calloc(size_t nmemb, size_t size);
    static void free(void* ptr);

  private:
    static char tmpbuf_[8192];
    static size_t tmppos_;
  };

}

#endif
