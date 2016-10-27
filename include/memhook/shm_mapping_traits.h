#ifndef MEMHOOK_INCLUDE_SHM_MAPPING_TRAITS_H_INCLUDED
#define MEMHOOK_INCLUDE_SHM_MAPPING_TRAITS_H_INCLUDED

#include <memhook/common.h>
#include <memhook/mapping_traits.h>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/noncopyable.hpp>

namespace memhook
{

typedef boost::interprocess::basic_managed_shared_memory<
        char,
        boost::interprocess::rbtree_best_fit<boost::interprocess::null_mutex_family>,
        boost::interprocess::iset_index
    > SHMNoLock;

typedef MappingTraits<
        SHMNoLock
    > SHMMappingTraits;

template <>
struct MappingCleaner<SHMMappingTraits> : private noncopyable
{
    std::string name_;
    explicit MappingCleaner(const char *name) : name_(name)
    {
        boost::interprocess::shared_memory_object::remove(name_.c_str());
    }

    ~MappingCleaner()
    {
        boost::interprocess::shared_memory_object::remove(name_.c_str());
    }
};

} // memhook

#endif
