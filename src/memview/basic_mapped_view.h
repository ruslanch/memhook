#ifndef MEMHOOK_BASIC_MAPPED_VIEW_H_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_H_INCLUDED

#include "common.h"
#include "interprocess_scoped_lock.h"
#include "mapped_view.h"
#include "mapped_view_base.h"

#include <memhook/mapping_traits.h>

#include <boost/interprocess/creation_tags.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/algorithm.hpp>

#include <iomanip>

namespace memhook {
  template <typename Traits, typename ContainerT = MappedContainer<Traits> >
  class BasicMappedView : public MappedViewBase {
  public:
    typedef typename Traits::Segment Segment;
    typedef ContainerT ConcreteMappedContainer;
    typedef typename ConcreteMappedContainer::IndexedContainer IndexedContainer;
    typedef typename ConcreteMappedContainer::SymbolTable SymbolTable;

    explicit BasicMappedView(const char *name)
        : m_segment(boost::interprocess::open_only, name)
        , m_container(m_segment.template find<ConcreteMappedContainer>(
                  MEMHOOK_SHARED_CONTAINER).first) {}

    void Write(std::ostream &os) {
      InterprocessScopedLock lock(*m_container, GetOptionFlag(kNoLock));
      WriteImpl(os);
    }

    std::size_t GetSize() {
      InterprocessScopedLock lock(*m_container, GetOptionFlag(kNoLock));
      return m_segment.get_size();
    }

    std::size_t GetFreeMemory() {
      InterprocessScopedLock lock(*m_container, GetOptionFlag(kNoLock));
      return m_segment.get_free_memory();
    }

    UniqueCharBuf GetModulePath(uintptr_t addr) const {
      typename SymbolTable::const_iterator iter = m_container->shltab.find(addr);
      if (BOOST_UNLIKELY(iter == m_container->shltab.end()))
        return UniqueCharBuf("<unknown>", UniqueCharBufNoFree);
      return UniqueCharBuf(iter->second.c_str(), UniqueCharBufNoFree);
    }

    UniqueCharBuf GetProcName(uintptr_t ip) const {
      typename SymbolTable::const_iterator iter = m_container->symtab.find(ip);
      if (BOOST_UNLIKELY(iter == m_container->symtab.end()))
        return UniqueCharBuf("<unknown>", UniqueCharBufNoFree);
      return UniqueCharBuf(iter->second.c_str(), UniqueCharBufNoFree);
    }

  protected:
    virtual void WriteImpl(std::ostream &os) = 0;

    IndexedContainer &GetIndexedContainer() {
      return m_container->indexed_container;
    }

  private:
    Segment m_segment;
    ConcreteMappedContainer *m_container;
  };
}

#endif
