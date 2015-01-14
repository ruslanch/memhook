#ifndef MEMHOOK_MAPPED_FILE_CONTEXT_HPP_INCLUDED
#define MEMHOOK_MAPPED_FILE_CONTEXT_HPP_INCLUDED

#include "mapped_file_traits.hpp"
#include "basic_mapped_storage.hpp"

namespace memhook {

typedef basic_mapped_storage<
        mapped_file_traits
    > mapped_file_storage;

} // memhook

#endif // MEMHOOK_MAPPED_FILE_CONTEXT_HPP_INCLUDED
