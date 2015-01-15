#ifndef MEMHOOK_MAPPED_FILE_TRAITS_HPP_INCLUDED
#define MEMHOOK_MAPPED_FILE_TRAITS_HPP_INCLUDED

#include "mapping_traits.hpp"
#include <boost/interprocess/managed_mapped_file.hpp>

namespace memhook {
typedef interprocess::basic_managed_mapped_file<
        char,
        interprocess::rbtree_best_fit<interprocess::null_mutex_family>,
        interprocess::iset_index
    > managed_mapped_file_no_lock;

typedef mapping_traits<
        managed_mapped_file_no_lock
    > mapped_file_traits;

template <>
struct mapping_cleaner<mapped_file_traits> : private noncopyable
{
    explicit mapping_cleaner(const char *name) {
        interprocess::file_mapping::remove(name);
    }
};

} // memhook

#endif // MEMHOOK_MAPPED_FILE_TRAITS_HPP_INCLUDED