#include "mapped_file_storage.hpp"

namespace memhook {

mapped_storage *make_mmf_storage(const char *path, std::size_t size) {
    return new mapped_file_storage(path, size);
}

} // memhook
