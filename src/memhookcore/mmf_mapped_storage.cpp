#include "basic_mapped_storage.h"
#include "basic_mapped_storage_creator.h"

#include <memhook/mmf_mapping_traits.h>

namespace memhook {
  typedef BasicMappedStorage<MMFMappingTraits,
          MappedContainer<MMFMappingTraits>
        > MMFMappedStorage;

  unique_ptr<MappedStorage> NewMMFMappedStorage(const char *path, std::size_t size) {
    return unique_ptr<MappedStorage>(new MMFMappedStorage(
            MEMHOOK_SHARED_CONTAINER, path, size));
  }

  unique_ptr<MappedStorageCreator> NewMMFMappedStorageCreator(const char *path, std::size_t size) {
    return unique_ptr<MappedStorageCreator>(new BasicMappedStorageCreator<MMFMappedStorage>(
            MEMHOOK_SHARED_CONTAINER, path, size));
  }

}
