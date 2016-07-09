#ifndef MEMHOOK_TRACEINFO_HPP_INCLUDED
#define MEMHOOK_TRACEINFO_HPP_INCLUDED

#include "common.hpp"
#include "callstack.hpp"
#include <boost/cstdint.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/adapt_adt.hpp>

namespace memhook {

struct traceinfo_base {
    traceinfo_base()
            : address(), memsize(), timestamp() {}

    traceinfo_base(uintptr_t address, size_t memsize, const boost::chrono::system_clock::time_point &timestamp)
            : address(address), memsize(memsize), timestamp(timestamp) {}

    uintptr_t   address;
    std::size_t memsize;
    boost::chrono::system_clock::time_point timestamp;
};

struct traceinfo_callstack_item {
    traceinfo_callstack_item()
        : shl_addr()
        , ip()
        , sp()
        , offp()
    {}

    traceinfo_callstack_item(uintptr_t shl_addr, uintptr_t ip, uintptr_t sp, uintptr_t offp)
        : shl_addr(shl_addr)
        , ip(ip)
        , sp(sp)
        , offp(offp)
    {}

    uintptr_t shl_addr;
    uintptr_t ip;
    uintptr_t sp;
    uintptr_t offp;
};

struct traceinfo_callstack_item_builder {
    traceinfo_callstack_item operator()(const callstack_record &r) const {
        return traceinfo_callstack_item(r.shl_addr, r.ip, r.sp, r.offp);
    }
};

template <typename Allocator = std::allocator<traceinfo_callstack_item> >
struct basic_traceinfo : traceinfo_base {
    typedef Allocator allocator_type;
    typedef boost::container::vector<traceinfo_callstack_item, allocator_type> callstack_type;

    basic_traceinfo(const allocator_type &allocator = allocator_type())
            : traceinfo_base()
            , callstack(allocator) {}

    basic_traceinfo(traceinfo_base &traceinfo, const allocator_type &allocator = allocator_type())
            : traceinfo_base(traceinfo)
            , callstack(allocator) {}

    basic_traceinfo(uintptr_t address, size_t memsize, const boost::chrono::system_clock::time_point &timestamp,
                const callstack_container &a_callstack, const allocator_type &allocator = allocator_type())
            : traceinfo_base(address, memsize, timestamp)
            , callstack(allocator) {
        callstack.reserve(a_callstack.size());
        boost::transform(a_callstack, std::back_inserter(callstack), traceinfo_callstack_item_builder());
    }

    callstack_type callstack;
};

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::traceinfo_base,
    (uintptr_t,   address)
    (std::size_t, memsize)
    (boost::chrono::system_clock::time_point, timestamp)
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::traceinfo_callstack_item,
    (uintptr_t, shl_addr)
    (uintptr_t, ip)
    (uintptr_t, sp)
    (uintptr_t, offp)
);

BOOST_FUSION_ADAPT_TPL_STRUCT(
    (Allocator),
    (memhook::basic_traceinfo)(Allocator),
    (uintptr_t,   address)
    (std::size_t, memsize)
    (boost::chrono::system_clock::time_point, timestamp)
    (typename memhook::basic_traceinfo<Allocator>::callstack_type, callstack)
);

#endif // MEMHOOK_TRACEINFO_HPP_INCLUDED
