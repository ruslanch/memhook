#ifndef MEMHOOK_SHARED_MEMORY_STORAGE_HPP_INCLUDED
#define MEMHOOK_SHARED_MEMORY_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/shared_memory_traits.hpp>
#include "basic_mapped_storage.hpp"

namespace memhook {

typedef basic_mapped_storage<
        shared_memory_traits
    > shared_memory_storage;

} // memhook

#endif // MEMHOOK_SHARED_MEMORY_STORAGE_HPP_INCLUDED
