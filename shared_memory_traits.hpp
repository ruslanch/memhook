#ifndef MEMHOOK_SHARED_MEMORY_TRAITS_HPP_INCLUDED
#define MEMHOOK_SHARED_MEMORY_TRAITS_HPP_INCLUDED

#include "mapping_traits.hpp"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/noncopyable.hpp>

namespace memhook {
typedef interprocess::basic_managed_shared_memory<
        char,
        interprocess::rbtree_best_fit<interprocess::null_mutex_family>,
        interprocess::iset_index
    > managed_shared_memory_no_lock;

typedef mapping_traits<
        managed_shared_memory_no_lock
    > shared_memory_traits;

template <>
struct mapping_cleaner<shared_memory_traits> : private noncopyable
{
    std::string name_;
    explicit mapping_cleaner(const char *name) : name_(name) {
        interprocess::shared_memory_object::remove(name_.c_str());
    }
    ~mapping_cleaner() {
        interprocess::shared_memory_object::remove(name_.c_str());
    }
};

} // memhook

#endif // MEMHOOK_SHARED_MEMORY_TRAITS_HPP_INCLUDED
