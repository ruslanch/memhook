#ifndef MEMHOOK_INCLUDE_NETWORK_H_INCLUDED
#define MEMHOOK_INCLUDE_NETWORK_H_INCLUDED

#include <memhook/common.h>
#include <memhook/traceinfo.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/unordered_map.hpp>

namespace memhook
{

enum NetReqType
{
    NetReqUnknown    = 0,
    NetReqInsert     = 0xc5cc9ef5,
    NetReqErase      = 0x261979c2,
    NetReqUpdateSize = 0x92587e96,
    NetReqFetch      = 0x82e96343,
    NetReqFetchEnd   = 0x02ad0248,
    NetReqEnd        = 0x00fc33b1,
    NetReqClear      = 0xe5b1f106,
};

enum NetResType
{
    NetResUnknown           = 0,
    NetResEnd               = 0x00fc33b1,
    NetResMoreDataAvailable = 0x45011baa,
};

struct NetProtoTag
{};

struct NetProtoOutbound : NetProtoTag
{
    NetProtoOutbound()
        : size(0)
    {}

    explicit NetProtoOutbound(std::size_t size)
        : size(size)
    {}

    std::size_t size;
};

struct NetRequest : NetProtoTag
{
    NetRequest()
            : type(NetReqUnknown)
            , traceinfo()
            , callstack()
    {}

    NetRequest(NetReqType type, uintptr_t address, std::size_t memsize,
                const boost::chrono::system_clock::time_point &timestamp,
                const CallStackInfo &a_callstack)
            : type(type)
            , traceinfo(address, memsize, timestamp)
            , callstack(a_callstack)
    {}

    NetReqType  type;
    TraceInfoBase traceinfo;
    CallStackInfo callstack;
};

struct NetResponse : NetProtoTag
{
    NetResponse()
            : req_type(NetReqUnknown)
            , response(NetResUnknown)
            , traceinfo()
            , symtab()
            , shltab()
    {}

    NetReqType req_type;
    NetResType response;

    typedef boost::container::vector<BasicTraceInfo<> > TraceInfoArray;
    TraceInfoArray traceinfo;

    typedef boost::unordered_map<uintptr_t, boost::container::string> SymbolTable;
    SymbolTable symtab;
    SymbolTable shltab;
};

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::NetProtoOutbound,
    (std::size_t, size)
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::NetRequest,
    (memhook::NetReqType,    type)
    (memhook::TraceInfoBase, traceinfo)
    (memhook::CallStackInfo, callstack)
);

BOOST_FUSION_ADAPT_STRUCT(
    memhook::NetResponse,
    (memhook::NetReqType, req_type)
    (memhook::NetResType, response)
    (memhook::NetResponse::TraceInfoArray, traceinfo)
    (memhook::NetResponse::SymbolTable,    symtab)
    (memhook::NetResponse::SymbolTable,    shltab)
);

#endif // MEMHOOK_NETWORK_H_INCLUDED
