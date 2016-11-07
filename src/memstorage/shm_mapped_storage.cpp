#include "shm_mapped_storage.h"

namespace memhook
{

unique_ptr<MappedStorage> NewSHMMappedStorage(const char *name, std::size_t size)
{
    return unique_ptr<MappedStorage>(new SHMMappedStorage(name, size));
}

} // memhook
