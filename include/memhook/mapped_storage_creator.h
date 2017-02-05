#ifndef MEMHOOK_INCLUDE_MAPPED_STORAGE_CREATOR_H_INCLUDED
#define MEMHOOK_INCLUDE_MAPPED_STORAGE_CREATOR_H_INCLUDED

#include <memhook/common.h>
#include <memhook/mapped_storage.h>

namespace memhook {
  class MappedStorageCreator : noncopyable {
  public:
    virtual ~MappedStorageCreator() {}
    virtual unique_ptr<MappedStorage> New(uintptr_t guide = 0) const = 0;
  };

  unique_ptr<MappedStorageCreator> NewMMFMappedStorageCreator(const char *path, std::size_t size);
  unique_ptr<MappedStorageCreator> NewSHMMappedStorageCreator(const char *path, std::size_t size);
  unique_ptr<MappedStorageCreator> NewNetworkMappedStorageCreator(const char *host, int port);
  unique_ptr<MappedStorageCreator> NewLevelDBMappedStorageCreator(const char *path);
}

#endif
