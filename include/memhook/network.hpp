#ifndef MEMHOOK_NETWORK_HPP_INCLUDED
#define MEMHOOK_NETWORK_HPP_INCLUDED

#include "common.hpp"
#include "traceinfo.hpp"
#include <boost/optional.hpp>

namespace memhook {

enum net_req_type {
    net_req_unspecified = 0,
    net_req_insert,
    net_req_erase,
    net_req_upd_size,
};

struct net_req {
    net_req()
            : type(net_req_unspecified)
            , traceinfo() {}

    net_req(net_req_type type, uintptr_t address, std::size_t memsize, const time_point_t &timestamp,
                const callstack_container &callstack)
            : type(type)
            , traceinfo(address, memsize, timestamp, callstack) {}

    net_req_type type;
    basic_traceinfo<> traceinfo;
};

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::net_req,
    (memhook::net_req_type, type)
    (memhook::basic_traceinfo<>, traceinfo)
);

#endif // MEMHOOK_NETWORK_HPP_INCLUDED
