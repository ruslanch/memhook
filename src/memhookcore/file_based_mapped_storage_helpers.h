#ifndef MEMHOOK_SRC_MEMHOOKCORE_FILE_BASED_MAPPED_STORAGE_HELPERS_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOKCORE_FILE_BASED_MAPPED_STORAGE_HELPERS_H_INCLUDED

#include "common.h"

namespace memhook {
  class FileBasedMappedStorageCreatorMixin {
  public:
    std::string GenerateUniquePath(const std::string &original_path, uintptr_t guide) const;
  };
}
#endif
