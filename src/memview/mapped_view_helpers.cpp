#include "mapped_view_helpers.h"

#include <cxxabi.h>

namespace memhook {
  UniqueCharBuf CxxSymbolDemangle(const char *source) {
    int ret = 0;
    UniqueCharBuf res(abi::__cxa_demangle(source, NULL, NULL, &ret), UniqueCharBufFree);
    if (ret == 0 && res)
      return boost::move(res);
    return UniqueCharBuf(source, UniqueCharBufNoFree);
  }
}
