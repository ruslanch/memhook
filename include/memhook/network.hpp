#ifndef MEMHOOK_NETWORK_HPP_INCLUDED
#define MEMHOOK_NETWORK_HPP_INCLUDED

#include "common.hpp"
#include "traceinfo.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/unordered_map.hpp>

namespace memhook {

enum net_req_type {
    net_req_unknown   = 0,
    net_req_insert    = 0xc5cc9ef5,
    net_req_erase     = 0x261979c2,
    net_req_upd_size  = 0x92587e96,
    net_req_fetch     = 0x82e96343,
    net_req_fetch_end = 0x02ad0248,
    net_req_end       = 0x00fc33b1,
};

enum net_res_type {
    net_res_unknown             = 0,
    net_res_end                 = 0x00fc33b1,
    net_res_more_data_available = 0x45011baa,
};

struct net_proto_tag {};

struct net_proto_outbound : net_proto_tag {
    net_proto_outbound()
        : size(0) {}

    explicit net_proto_outbound(std::size_t size)
        : size(size) {}

    std::size_t size;
};

struct net_req : net_proto_tag {
    net_req()
            : type(net_req_unknown)
            , traceinfo()
            , callstack() {}

    net_req(net_req_type type, uintptr_t address, std::size_t memsize,
                const system_clock::time_point &timestamp, callstack_container &a_callstack)
            : type(type)
            , traceinfo(address, memsize, timestamp)
            , callstack() {
        callstack.swap(a_callstack);
    }

    net_req_type        type;
    traceinfo_base      traceinfo;
    callstack_container callstack;
};

struct net_response : net_proto_tag {
    net_response()
            : req_type(net_req_unknown)
            , response(net_res_unknown)
            , traceinfo_vec()
            , symtab()
            , shltab() {}

    net_req_type req_type;
    net_res_type response;

    typedef container::vector<basic_traceinfo<> > traceinfo_vec_t;
    traceinfo_vec_t traceinfo_vec;

    typedef unordered_map<uintptr_t, container::string> symbol_table_t;
    symbol_table_t symtab;
    symbol_table_t shltab;
};

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::net_proto_outbound,
    (std::size_t, size)
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::net_req,
    (memhook::net_req_type,        type)
    (memhook::traceinfo_base,      traceinfo)
    (memhook::callstack_container, callstack)
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::net_response,
    (memhook::net_req_type, req_type)
    (memhook::net_res_type, response)
    (memhook::net_response::traceinfo_vec_t, traceinfo_vec)
    (memhook::net_response::symbol_table_t,  symtab)
    (memhook::net_response::symbol_table_t,  shltab)
);

#endif // MEMHOOK_NETWORK_HPP_INCLUDED
