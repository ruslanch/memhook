#ifndef MEMHOOK_SHARED_MEMORY_STORAGE_IPP_INCLUDED
#define MEMHOOK_SHARED_MEMORY_STORAGE_IPP_INCLUDED

#include "shared_memory_storage.hpp"

namespace memhook {

mapped_storage *make_shared_memory_storage(const char *name, std::size_t size) {
    return new shared_memory_storage(name, size);
}

} // memhook

#endif // MEMHOOK_SHARED_MEMORY_STORAGE_IPP_INCLUDED
