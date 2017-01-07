#ifndef MEMHOOK_INCLUDE_SHM_MAPPING_TRAITS_H_INCLUDED
#define MEMHOOK_INCLUDE_SHM_MAPPING_TRAITS_H_INCLUDED

#include <memhook/common.h>
#include <memhook/mapping_traits.h>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/noncopyable.hpp>

namespace memhook {
  typedef boost::interprocess::basic_managed_shared_memory<
          char,
          boost::interprocess::rbtree_best_fit<boost::interprocess::null_mutex_family>,
          boost::interprocess::iset_index
        > SHMNoLock;

  typedef MappingTraits<SHMNoLock> SHMMappingTraits;

  template <>
  class MappingCleaner<SHMMappingTraits> : noncopyable {
  public:
    explicit MappingCleaner(const char *name) : m_name(name) {
      boost::interprocess::shared_memory_object::remove(m_name.c_str());
    }

    ~MappingCleaner() {
      boost::interprocess::shared_memory_object::remove(m_name.c_str());
    }
  private:
    std::string m_name;
  };
}  // memhook

#endif
