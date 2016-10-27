#ifndef MEMHOOK_BASIC_MAPPED_VIEW_H_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_H_INCLUDED

#include "common.h"
#include "mapped_view.h"
#include "mapped_view_base.h"
#include "interprocess_scoped_lock.h"

#include <memhook/mapping_traits.h>

#include <boost/interprocess/creation_tags.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/noncopyable.hpp>

#include <iomanip>

namespace memhook
{

template <typename Traits>
class BasicMappedView : public MappedViewBase
{
public:
    typedef typename Traits::segment Segment;
    typedef MappedContainer<Traits> ConcreteMappedContainer;
    typedef typename ConcreteMappedContainer::IndexedContainer IndexedContainer;
    typedef typename ConcreteMappedContainer::SymbolTable      SymbolTable;

    explicit BasicMappedView(const char *name);

    void Write(std::ostream &os);

    std::size_t GetSize();
    std::size_t GetFreeMemory();

    UniqueCharBuf GetModulePath(uintptr_t addr) const;
    UniqueCharBuf GetProcName(uintptr_t ip) const;

protected:
    virtual void WriteImpl(std::ostream &os) = 0;

    IndexedContainer &get_indexed_container()
    {
        return container->indexed_container;
    }

private:
    Segment segment;
    ConcreteMappedContainer *container;
};

template <typename Traits>
BasicMappedView<Traits>::BasicMappedView(const char *name)
    : segment(boost::interprocess::open_only, name)
    , container(segment.template find<ConcreteMappedContainer>(MEMHOOK_SHARED_CONTAINER).first)
{}

template <typename Traits>
void BasicMappedView<Traits>::Write(std::ostream &os)
{
    InterprocessScopedLock lock(*container, GetOptionFlag(NoLock));
    WriteImpl(os);
}

template <typename Traits>
std::size_t BasicMappedView<Traits>::GetSize()
{
    InterprocessScopedLock lock(*container, GetOptionFlag(NoLock));
    return segment.get_size();
}

template <typename Traits>
std::size_t BasicMappedView<Traits>::GetFreeMemory()
{
    InterprocessScopedLock lock(*container, GetOptionFlag(NoLock));
    return segment.get_free_memory();
}

template <typename Traits>
UniqueCharBuf BasicMappedView<Traits>::GetModulePath(uintptr_t shl_addr) const
{
    typename SymbolTable::const_iterator iter = container->shltab.find(shl_addr);
    if (iter != container->shltab.end())
        return UniqueCharBuf(iter->second.c_str(), UniqueCharBufNoFree);
    return UniqueCharBuf("<unknown>", UniqueCharBufNoFree);
}

template <typename Traits>
UniqueCharBuf BasicMappedView<Traits>::GetProcName(uintptr_t ip) const
{
    typename SymbolTable::const_iterator iter = container->symtab.find(ip);
    if (iter != container->symtab.end())
        return UniqueCharBuf(iter->second.c_str(), UniqueCharBufNoFree);
    return UniqueCharBuf("<unknown>", UniqueCharBufNoFree);
}

} // memhook

#endif
