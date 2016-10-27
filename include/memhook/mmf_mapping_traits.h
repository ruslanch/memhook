#ifndef MEMHOOK_INCLUDE_MMF_MAPPING_TRAITS_H_INCLUDED
#define MEMHOOK_INCLUDE_MMF_MAPPING_TRAITS_H_INCLUDED

#include <memhook/common.h>
#include <memhook/mapping_traits.h>

#include <boost/interprocess/managed_mapped_file.hpp>

namespace memhook
{

typedef boost::interprocess::basic_managed_mapped_file<
        char,
        boost::interprocess::rbtree_best_fit<boost::interprocess::null_mutex_family>,
        boost::interprocess::iset_index
    > MMFNoLock;

typedef MappingTraits<
        MMFNoLock
    > MMFMappingTraits;

template <>
struct MappingCleaner<MMFMappingTraits> : private boost::noncopyable
{
    explicit MappingCleaner(const char *name)
    {
        boost::interprocess::file_mapping::remove(name);
    }
};

} // memhook

#endif
