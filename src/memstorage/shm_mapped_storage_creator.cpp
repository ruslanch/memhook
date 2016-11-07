#include "common.h"
#include "shm_mapped_storage.h"
#include "basic_mapped_storage_creator.h"

namespace memhook
{

unique_ptr<MappedStorageCreator> NewSHMMappedStorageCreator(const char *path, std::size_t size)
{
    return unique_ptr<MappedStorageCreator>(new BasicMappedStorageCreator<SHMMappedStorage>(path, size));
}

} // ns memhook
