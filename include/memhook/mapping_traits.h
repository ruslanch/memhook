#ifndef MEMHOOK_INCLUDE_MAPPING_TRAITS_H_INCLUDED
#define MEMHOOK_INCLUDE_MAPPING_TRAITS_H_INCLUDED

#include <memhook/common.h>
#include <memhook/callstack.h>
#include <memhook/traceinfo.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/cstdint.hpp>

namespace memhook
{

template <typename Segment>
struct MappingTraits
{
    typedef Segment                                               segment;
    typedef typename Segment::segment_manager                     segment_manager;
    typedef boost::interprocess::allocator<void, segment_manager> generic_allocator;
};

template <typename Traits>
struct MappedTraceInfoTypes
{
    typedef BasicTraceInfo<
            boost::interprocess::allocator<
                TraceInfoCallStackItem,
                typename Traits::segment_manager
            >
        > base_type;
};

template <typename Traits>
struct MappedTraceInfo
        : BasicTraceInfo<
                boost::interprocess::allocator<TraceInfoCallStackItem, typename Traits::segment_manager>
            >
{
    typedef BasicTraceInfo<
            boost::interprocess::allocator<TraceInfoCallStackItem, typename Traits::segment_manager>
        > base_type;

    typedef typename Traits::generic_allocator generic_allocator;

    explicit MappedTraceInfo(const generic_allocator &allocator_instance)
        : base_type(allocator_instance)
    {}

    MappedTraceInfo(uintptr_t address, size_t memsize,
            const boost::chrono::system_clock::time_point &timestamp,
            const CallStackInfo &callstack, const generic_allocator &allocator_instance)
        : base_type(address, memsize, timestamp, callstack, allocator_instance)
    {}
};

template <typename Traits, typename CharT = char>
struct MappedContainer : boost::interprocess::interprocess_mutex {
    typedef typename Traits::segment_manager   segment_manager;
    typedef typename Traits::generic_allocator generic_allocator;
    typedef MappedTraceInfo<Traits>            TraceInfo;

    typedef boost::multi_index_container<
        TraceInfo,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::member<TraceInfoBase,
                    uintptr_t, &TraceInfoBase::address
                >
            >,
            boost::multi_index::ordered_non_unique<
                boost::multi_index::member<TraceInfoBase,
                    boost::chrono::system_clock::time_point, &TraceInfoBase::timestamp
                >
            >,
            boost::multi_index::ordered_non_unique<
                boost::multi_index::member<TraceInfoBase,
                    std::size_t, &TraceInfoBase::memsize
                >
            >
        >,
        boost::interprocess::allocator<TraceInfo, segment_manager>
    > IndexedContainer;

    IndexedContainer indexed_container;

    typedef boost::container::basic_string<
            CharT,
            std::char_traits<CharT>,
            boost::interprocess::allocator<CharT, segment_manager>
        > String;

    typedef std::pair<const uintptr_t, String> SymbolTableValue;

    typedef boost::unordered_map<
            uintptr_t,
            String,
            boost::hash<uintptr_t>,
            std::equal_to<uintptr_t>,
            boost::interprocess::allocator<SymbolTableValue, segment_manager>
        > SymbolTable;

    SymbolTable symtab;
    SymbolTable shltab;

    explicit MappedContainer(const generic_allocator &allocator_instance)
        : indexed_container(typename IndexedContainer::ctor_args_list(), allocator_instance)
        , symtab(allocator_instance)
        , shltab(allocator_instance)
    {}
};

template <typename Traits>
struct MappingCleaner;

} // memhook

#endif
