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
            : address_(), memsize_(), timestamp_() {}

    traceinfo_base(uintptr_t address, size_t memsize, const system_clock::time_point &timestamp)
            : address_(address), memsize_(memsize), timestamp_(timestamp) {}

    uintptr_t address() const { return address_; }
    void address(uintptr_t a_address) { address_ = a_address; }

    std::size_t memsize() const { return memsize_; }
    void memsize(std::size_t a_memsize) { memsize_ = a_memsize; }

    system_clock::time_point timestamp() const { return timestamp_; }
    void timestamp(const system_clock::time_point &a_timestamp) { timestamp_ = a_timestamp; }

private:
    uintptr_t address_;
    std::size_t memsize_;
    system_clock::time_point timestamp_;
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
    typedef container::vector<traceinfo_callstack_item, allocator_type> callstack_type;

    basic_traceinfo(const allocator_type &allocator = allocator_type())
            : callstack_(allocator) {}

    basic_traceinfo(uintptr_t address, size_t memsize, const system_clock::time_point &timestamp,
                const callstack_container &callstack, const allocator_type &allocator = allocator_type())
            : traceinfo_base(address, memsize, timestamp)
            , callstack_(allocator) {
        callstack_.reserve(callstack.size());
        transform(callstack, std::back_inserter(callstack_), traceinfo_callstack_item_builder());
    }

    const callstack_type &callstack() const {
        return callstack_;
    }

    void callstack(const callstack_type &a_callstack) {
        callstack_ = a_callstack;
    }

private:
    callstack_type callstack_;
};

} // memhook

BOOST_FUSION_ADAPT_ADT(
    memhook::traceinfo_base,
    (uintptr_t, uintptr_t, obj.address(), obj.address(val))
    (std::size_t, std::size_t, obj.memsize(), obj.memsize(val))
    (memhook::system_clock::time_point, memhook::system_clock::time_point, obj.timestamp(), obj.timestamp(val))
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::traceinfo_callstack_item,
    (uintptr_t, shl_addr)
    (uintptr_t, ip)
    (uintptr_t, sp)
    (uintptr_t, offp)
);

BOOST_FUSION_ADAPT_TPL_ADT(
    (Allocator),
    (memhook::basic_traceinfo)(Allocator),
    (uintptr_t, uintptr_t, obj.address(), obj.address(val))
    (std::size_t, std::size_t, obj.memsize(), obj.memsize(val))
    (memhook::system_clock::time_point, memhook::system_clock::time_point, obj.timestamp(), obj.timestamp(val))
    (const typename memhook::basic_traceinfo<Allocator>::callstack_type &,
     const typename memhook::basic_traceinfo<Allocator>::callstack_type &,
     obj.callstack(), obj.callstack(val))
);

#endif // MEMHOOK_TRACEINFO_HPP_INCLUDED
