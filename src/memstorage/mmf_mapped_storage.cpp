#include "mmf_mapped_storage.h"

namespace memhook
{

unique_ptr<MappedStorage> NewMMFMappedStorage(const char *path, std::size_t size)
{
    return unique_ptr<MappedStorage>(new MMFMappedStorage(path, size));
}

} // memhook
