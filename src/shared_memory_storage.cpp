#include "shared_memory_storage.hpp"

namespace memhook {

mapped_storage *make_shm_storage(const char *name, std::size_t size) {
    return new shared_memory_storage(name, size);
}

} // memhook
