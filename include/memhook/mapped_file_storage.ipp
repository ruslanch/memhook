#ifndef MEMHOOK_MAPPED_FILE_STORAGE_IPP_INCLUDED
#define MEMHOOK_MAPPED_FILE_STORAGE_IPP_INCLUDED

#include "mapped_file_storage.hpp"

namespace memhook {

mapped_storage *make_mapped_file_storage(const char *path, std::size_t size) {
    return new mapped_file_storage(path, size);
}

} // memhook

#endif // MEMHOOK_MAPPED_FILE_STORAGE_IPP_INCLUDED
