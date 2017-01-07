#ifndef MEMHOOK_SRC_MEMHOOKCORE_BASIC_MAPPED_STORAGE_CREATOR_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOKCORE_BASIC_MAPPED_STORAGE_CREATOR_H_INCLUDED

#include "common.h"
#include "file_based_mapped_storage_helpers.h"

#include <memhook/mapped_storage_creator.h>

namespace memhook {
  template <typename MappedStorageT>
  class BasicMappedStorageCreator : public MappedStorageCreator,
          private FileBasedMappedStorageCreatorMixin {
  public:
    BasicMappedStorageCreator(const char *container_name, const char *path, std::size_t size)
        : m_container_name(container_name)
        , m_path(path)
        , m_size(size) {}

    unique_ptr<MappedStorage> New(uintptr_t guide) const {
      return unique_ptr<MappedStorage>(new MappedStorageT(
              m_container_name.c_str(), GenerateUniquePath(m_path, guide).c_str(), m_size));
    }

  private:
    std::string m_container_name;
    std::string m_path;
    std::size_t m_size;
  };
}

#endif
