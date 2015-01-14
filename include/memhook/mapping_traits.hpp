#ifndef MEMHOOK_MAPPING_TRAITS_HPP_INCLUDED
#define MEMHOOK_MAPPING_TRAITS_HPP_INCLUDED

#include "common.hpp"
#include "callstack.hpp"
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>

namespace memhook {

template <typename Segment>
struct mapping_traits {
    typedef Segment                                        segment;
    typedef typename Segment::segment_manager              segment_manager;
    typedef interprocess::allocator<void, segment_manager> generic_allocator;
};

struct traceinfo_callstack_item {
    traceinfo_callstack_item(uintptr_t shl_addr, uintptr_t ip, uintptr_t sp, uintptr_t offp)
        : shl_addr(shl_addr), ip(ip), sp(sp), offp(offp)
    {}

    uintptr_t shl_addr;
    uintptr_t ip;
    uintptr_t sp;
    uintptr_t offp;
};

struct traceinfo_callstack_item_maker {
    traceinfo_callstack_item operator()(const callstack_record &r) const {
        return traceinfo_callstack_item(r.shl_addr, r.ip, r.sp, r.offp);
    }
};

template <typename Traits>
struct traceinfo : private noncopyable {
    typedef typename Traits::segment_manager   segment_manager;
    typedef typename Traits::generic_allocator generic_allocator;

    typedef interprocess::vector<
            traceinfo_callstack_item,
            interprocess::allocator<traceinfo_callstack_item, segment_manager>
        > traceinfo_callstack_container;

    explicit traceinfo(const generic_allocator &allocator_instance)
        : callstack(allocator_instance) {}

    traceinfo(uintptr_t address, size_t memsize, const system_clock_t::time_point &timestamp,
            const callstack_container &a_callstack, const generic_allocator &allocator_instance)
        : address(address)
        , memsize(memsize)
        , timestamp(timestamp)
        , callstack(allocator_instance)
    {
        callstack.reserve(a_callstack.size());
        transform(a_callstack, std::back_inserter(callstack),
            traceinfo_callstack_item_maker());
    }

    uintptr_t                     address;
    std::size_t                   memsize;
    system_clock_t::time_point    timestamp;
    traceinfo_callstack_container callstack;
};

template <typename Traits, typename CharT = char>
struct mapped_container : interprocess::interprocess_mutex {
    typedef typename Traits::segment_manager   segment_manager;
    typedef typename Traits::generic_allocator generic_allocator;
    typedef traceinfo<Traits>                  traceinfo_type;

    typedef multi_index_container<
        traceinfo_type,
        multi_index::indexed_by<
            multi_index::hashed_unique<
                multi_index::member<traceinfo_type,
                    uintptr_t, &traceinfo_type::address
                >
            >,
            multi_index::ordered_non_unique<
                multi_index::member<traceinfo_type,
                    system_clock_t::time_point, &traceinfo_type::timestamp
                >
            >,
            multi_index::ordered_non_unique<
                multi_index::member<traceinfo_type,
                    std::size_t, &traceinfo_type::memsize
                >
            >
        >,
        interprocess::allocator<traceinfo_type, segment_manager>
    > indexed_container_t;
    indexed_container_t indexed_container;

    typedef interprocess::basic_string<
            CharT,
            std::char_traits<CharT>,
            interprocess::allocator<CharT, segment_manager>
        > string_type;

    typedef uintptr_t   symbol_table_key_type;
    typedef string_type symbol_table_mapping_type;
    typedef std::pair<
            const symbol_table_key_type,
            symbol_table_mapping_type
        > symbol_table_value_type;

    typedef unordered_map<
            symbol_table_key_type,
            symbol_table_mapping_type,
            hash<symbol_table_key_type>,
            std::equal_to<symbol_table_key_type>,
            interprocess::allocator<symbol_table_value_type, segment_manager>
        > symbol_table_t;
    symbol_table_t symtab;
    symbol_table_t shltab;

    explicit mapped_container(const generic_allocator &allocator_instance)
        : indexed_container(typename indexed_container_t::ctor_args_list(), allocator_instance)
        , symtab(allocator_instance)
        , shltab(allocator_instance)
    {}
};

template <typename Traits>
struct mapping_cleaner;

} // memhook

#endif // MEMHOOK_MAPPING_TRAITS_HPP_INCLUDED
