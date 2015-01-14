#ifndef MEMHOOK_MAPPED_FILE_VIEW_HPP_INCLUDED
#define MEMHOOK_MAPPED_FILE_VIEW_HPP_INCLUDED

#include "mapped_file_traits.hpp"
#include "basic_mapped_view.hpp"
#include "basic_mapped_view_kit.hpp"
#include "basic_simple_mapped_view.hpp"

namespace memhook {

typedef basic_mapped_view_req<
        mapped_file_traits
    > mapped_file_view_req;

typedef basic_min_size_mapped_view_req<
        mapped_file_traits
    > min_size_mapped_file_view_req;

typedef basic_min_time_mapped_view_req<
        mapped_file_traits
    > min_time_mapped_file_view_req;

typedef basic_max_time_mapped_view_req<
        mapped_file_traits
    > max_time_mapped_file_view_req;

typedef basic_simple_mapped_view<
        mapped_file_traits
    > mapped_file_simple_view;

typedef basic_mapped_view_kit<
        mapped_file_traits
    > mapped_file_view_kit;

} // memhook

#endif // MEMHOOK_MAPPED_FILE_VIEW_HPP_INCLUDED
