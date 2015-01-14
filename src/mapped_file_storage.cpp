#include "mapped_file_storage.hpp"

namespace memhook {

mapped_storage *make_mmf_storage(const char *name, std::size_t size) {
    return new mapped_file_storage(name, size);
}

} // memhook
