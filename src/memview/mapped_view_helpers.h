#ifndef MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_HELPERS_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_HELPERS_H_INCLUDED

#include "common.h"

namespace memhook {
  typedef unique_ptr<const char, void (*)(const char *)> UniqueCharBuf;

  inline void UniqueCharBufNoFree(const char *)  {}
  inline void UniqueCharBufFree(const char *mem) { free(const_cast<char *>(mem)); }
  UniqueCharBuf CxxSymbolDemangle(const char *source);
}

#endif
