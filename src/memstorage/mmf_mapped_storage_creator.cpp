#include "common.h"
#include "mmf_mapped_storage.h"
#include "basic_mapped_storage_creator.h"

namespace memhook
{

unique_ptr<MappedStorageCreator> NewMMFMappedStorageCreator(const char *path, std::size_t size)
{
    return unique_ptr<MappedStorageCreator>(new BasicMappedStorageCreator<MMFMappedStorage>(path, size));
}

} // ns memhook
