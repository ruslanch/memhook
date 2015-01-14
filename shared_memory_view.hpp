#ifndef MEMHOOK_SHARED_MEMORY_VIEW_HPP_INCLUDED
#define MEMHOOK_SHARED_MEMORY_VIEW_HPP_INCLUDED

#include "shared_memory_traits.hpp"
#include "basic_mapped_view.hpp"
#include "basic_mapped_view_kit.hpp"
#include "basic_simple_mapped_view.hpp"

namespace memhook {

typedef basic_mapped_view_req<
        shared_memory_traits
    > shared_memory_view_req;

typedef basic_min_size_mapped_view_req<
        shared_memory_traits
    > min_size_shared_memory_view_req;

typedef basic_min_time_mapped_view_req<
        shared_memory_traits
    > min_time_shared_memory_view_req;

typedef basic_max_time_mapped_view_req<
        shared_memory_traits
    > max_time_shared_memory_view_req;

typedef basic_simple_mapped_view<
        shared_memory_traits
    > shared_memory_simple_view;

typedef basic_mapped_view_kit<
        shared_memory_traits
    > shared_memory_view_kit;

} // memhook

#endif // MEMHOOK_SHARED_MEMORY_VIEW_HPP_INCLUDED
