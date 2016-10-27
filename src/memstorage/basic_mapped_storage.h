#ifndef MEMHOOK_INCLUDE_BASIC_MAPPED_STORAGE_H_INCLUDED
#define MEMHOOK_INCLUDE_BASIC_MAPPED_STORAGE_H_INCLUDED

#include <memhook/common.h>
#include <memhook/mapping_traits.h>
#include <memhook/scoped_signal.h>
#include <memhook/mapped_storage.h>

#include <boost/interprocess/creation_tags.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace memhook
{

template <typename Traits>
struct BasicMappedStorage : MappedStorage
{
    typedef MappingCleaner<Traits> Cleaner;
    Cleaner cleaner;

    typedef typename Traits::segment Segment;
    Segment segment;

    typedef typename Traits::generic_allocator GenericAllocator;
    GenericAllocator allocator_instance;

    typedef MappedContainer<Traits> ConcreteMappedContainer;
    ConcreteMappedContainer *container;

    typedef typename ConcreteMappedContainer::IndexedContainer IndexedContainer;
    typedef typename ConcreteMappedContainer::SymbolTable      SymbolTable;
    typedef typename ConcreteMappedContainer::String           String;

    typedef MappedTraceInfo<Traits> TraceInfo;

    BasicMappedStorage(const char *name, std::size_t size)
        : cleaner(name)
        , segment(boost::interprocess::create_only, name, size)
        , allocator_instance(segment.get_segment_manager())
        , container(segment.template construct<ConcreteMappedContainer>(
            MEMHOOK_SHARED_CONTAINER)(allocator_instance))
    {}

    void Insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        const CallStackInfo &callstack);
    bool Erase(uintptr_t address);
    bool UpdateSize(uintptr_t address, std::size_t memsize);
    void Clear();

private:
    struct SymbolTableUpdater
    {
        ConcreteMappedContainer  &container;
        GenericAllocator         &allocator_instance;

        explicit SymbolTableUpdater(ConcreteMappedContainer &container,
                GenericAllocator &allocator_instance)
            : container(container)
            , allocator_instance(allocator_instance)
        {}

        bool operator()(const CallStackInfoItem &item)
        {
            typename SymbolTable::const_iterator iter = container.symtab.find(item.ip);
            if (iter == container.symtab.end())
            {
                String symbol(item.procname.begin(), item.procname.end(), allocator_instance);
                container.symtab.emplace(item.ip, boost::move(symbol));

                iter = container.shltab.find(item.shl_addr);
                if (iter == container.shltab.end())
                {
                    String shl_path(item.shl_path.begin(), item.shl_path.end(),
                        allocator_instance);
                    container.shltab.emplace(item.shl_addr, boost::move(shl_path));
                }
            }
            return false;
        }
    };
};

template <typename Traits>
void BasicMappedStorage<Traits>::Insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        const CallStackInfo &callstack)
{
    ScopedSignalLock signal_lock;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    if (boost::find_if(callstack, SymbolTableUpdater(*container, allocator_instance)) != callstack.end())
        return;
     container->indexed_container.emplace(address, memsize, timestamp, callstack,
        allocator_instance);
}

template <typename Traits>
bool BasicMappedStorage<Traits>::Erase(uintptr_t address)
{
    ScopedSignalLock signal_lock;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    typedef typename IndexedContainer::template nth_index<0>::type index0;
    index0 &idx = boost::get<0>(container->indexed_container);
    const typename index0::iterator iter = idx.find(address);
    const bool ret = iter != idx.end();
    if (ret)
        idx.erase(iter);
    return ret;
}

template <typename Traits>
bool BasicMappedStorage<Traits>::UpdateSize(uintptr_t address, std::size_t memsize)
{
    ScopedSignalLock signal_lock;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    typedef typename IndexedContainer::template nth_index<0>::type index0;
    index0 &idx = boost::get<0>(container->indexed_container);
    const typename index0::iterator iter = idx.find(address);
    const bool ret = iter != idx.end();
    if (ret)
        idx.modify(iter, boost::lambda::bind(&TraceInfoBase::memsize, boost::lambda::_1) = memsize);
    return ret;
}

template <typename Traits>
void BasicMappedStorage<Traits>::Clear()
{
    ScopedSignalLock signal_lock;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    container->indexed_container.clear();
}

} // memhook

#endif
