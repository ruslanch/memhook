#ifndef MEMHOOK_INCLUDE_TRACE_INFO_H_INCLUDED
#define MEMHOOK_INCLUDE_TRACE_INFO_H_INCLUDED

#include <memhook/common.h>
#include <memhook/callstack.h>

#include <boost/cstdint.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/adapt_adt.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/container/vector.hpp>

namespace memhook
{

struct TraceInfoBase
{
    TraceInfoBase()
            : address(), memsize(), timestamp()
    {}

    TraceInfoBase(uintptr_t address, size_t memsize, const boost::chrono::system_clock::time_point &timestamp)
            : address(address), memsize(memsize), timestamp(timestamp)
    {}

    uintptr_t   address;
    std::size_t memsize;
    boost::chrono::system_clock::time_point timestamp;
};

struct TraceInfoCallStackItem
{
    TraceInfoCallStackItem()
        : shl_addr()
        , ip()
        , sp()
        , offp()
    {}

    TraceInfoCallStackItem(uintptr_t shl_addr, uintptr_t ip, uintptr_t sp, uintptr_t offp)
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

struct TraceInfoCallStackItemBuilder
{
    TraceInfoCallStackItem operator()(const CallStackInfoItem &item) const
    {
        return TraceInfoCallStackItem(item.shl_addr, item.ip, item.sp, item.offp);
    }
};

template <typename AllocatorT = std::allocator<TraceInfoCallStackItem> >
struct BasicTraceInfo : TraceInfoBase
{
    typedef AllocatorT Allocator;
    typedef boost::container::vector<TraceInfoCallStackItem, Allocator> CallStack;

    BasicTraceInfo(const Allocator &allocator = Allocator())
            : TraceInfoBase()
            , callstack(allocator)
    {}

    BasicTraceInfo(TraceInfoBase &traceinfo, const Allocator &allocator = Allocator())
            : TraceInfoBase(traceinfo)
            , callstack(allocator)
    {}

    BasicTraceInfo(uintptr_t address, size_t memsize, const boost::chrono::system_clock::time_point &timestamp,
                const CallStackInfo &a_callstack, const Allocator &allocator = Allocator())
            : TraceInfoBase(address, memsize, timestamp)
            , callstack(allocator)
    {
        boost::transform(a_callstack, std::back_inserter(callstack), TraceInfoCallStackItemBuilder());
    }

    CallStack callstack;
};

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::TraceInfoBase,
    (uintptr_t,   address)
    (std::size_t, memsize)
    (boost::chrono::system_clock::time_point, timestamp)
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::TraceInfoCallStackItem,
    (uintptr_t, shl_addr)
    (uintptr_t, ip)
    (uintptr_t, sp)
    (uintptr_t, offp)
);

BOOST_FUSION_ADAPT_TPL_STRUCT(
    (AllocatorT),
    (memhook::BasicTraceInfo)(AllocatorT),
    (uintptr_t,   address)
    (std::size_t, memsize)
    (boost::chrono::system_clock::time_point, timestamp)
    (typename memhook::BasicTraceInfo<AllocatorT>::CallStack, callstack)
);

#endif
