#include "basic_mapped_storage.h"
#include "basic_mapped_storage_creator.h"

#include <memhook/shm_mapping_traits.h>

namespace memhook {
  typedef BasicMappedStorage<SHMMappingTraits,
          MappedContainer<SHMMappingTraits>
        > SHMMappedStorage;

  unique_ptr<MappedStorage> NewSHMMappedStorage(const char *name, std::size_t size) {
    return unique_ptr<MappedStorage>(new SHMMappedStorage(
            MEMHOOK_SHARED_CONTAINER, name, size));
  }

  unique_ptr<MappedStorageCreator> NewSHMMappedStorageCreator(const char *path, std::size_t size) {
    return unique_ptr<MappedStorageCreator>(new BasicMappedStorageCreator<SHMMappedStorage>(
            MEMHOOK_SHARED_CONTAINER, path, size));
  }
}
