#ifndef MEMHOOK_INCLUDE_NETWORK_H_INCLUDED
#define MEMHOOK_INCLUDE_NETWORK_H_INCLUDED

#include <memhook/common.h>
#include <memhook/traceinfo.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>

namespace memhook {
  enum NetReqType {
    kNetReqUnknown    = 0,
    kNetReqAdd        = 0xfd1a73e7,
    kNetReqRemove     = 0x68801d30,
    kNetReqUpdateSize = 0x23ac73f5,
    kNetReqFetch      = 0x82e96343,
    kNetReqFetchEnd   = 0x02ad0248,
    kNetReqEnd        = 0x00fc33b1,
    kNetReqClear      = 0xe5b1f106,
    kNetReqNewStorage = 0x8f3948da,
  };

  enum NetResType {
    kNetResUnknown           = 0,
    kNetResEnd               = 0x00fc33b1,
    kNetResMoreDataAvailable = 0x45011baa,
  };

  struct NetProtoTag {};

  struct NetProtoOutbound : NetProtoTag {
    NetProtoOutbound() : size(0) {}
    explicit NetProtoOutbound(std::size_t size) : size(size) {}
    std::size_t size;
  };

  struct NetRequest : NetProtoTag {
      NetRequest()
          : type(kNetReqUnknown)
          , traceinfo()
          , callstack() {}

      NetRequest(NetReqType type,
              uintptr_t address = 0,
              std::size_t memsize = 0,
              const chrono::system_clock::time_point &timestamp = chrono::system_clock::time_point(),
              const CallStackInfo &a_callstack = CallStackInfo())
          : type(type)
          , traceinfo(address, memsize, timestamp)
          , callstack(a_callstack) {}

      NetReqType    type;
      TraceInfoBase traceinfo;
      CallStackInfo callstack;
  };

  struct NetResponse : NetProtoTag {
    NetResponse()
        : req_type(kNetReqUnknown)
        , response(kNetResUnknown)
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
