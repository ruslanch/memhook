#ifndef MEMHOOK_NETWORK_HPP_INCLUDED
#define MEMHOOK_NETWORK_HPP_INCLUDED

#include "common.hpp"
#include "traceinfo.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

namespace memhook {

enum network_storage_req_type {
    net_req_unspecified = 0,
    net_req_insert,
    net_req_erase,
    net_req_upd_size
};

struct network_storage_req {
    network_storage_req()
            : type(net_req_unspecified)
            , traceinfo() {}

    network_storage_req(network_storage_req_type type, uintptr_t address, std::size_t memsize,
                const system_clock::time_point &timestamp,
                const callstack_container &callstack)
            : type(net_req_unspecified)
            , traceinfo(address, memsize, timestamp, callstack) {}

    network_storage_req_type type;
    basic_traceinfo<> traceinfo;
};

std::ostream &operator<<(std::ostream &os, const network_storage_req &req);
std::istream &operator>>(std::istream &is, network_storage_req &req);

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::network_storage_req,
    (memhook::network_storage_req_type, type)
    (memhook::basic_traceinfo<>, traceinfo)
);

#endif // MEMHOOK_NETWORK_HPP_INCLUDED
