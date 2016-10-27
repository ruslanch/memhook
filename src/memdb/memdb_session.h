#ifndef MEMHOOK_SRC_MEMDB_MEMDB_SESSION_H_INCLUDED
#define MEMHOOK_SRC_MEMDB_MEMDB_SESSION_H_INCLUDED

#include "common.h"

#include <memhook/network.h>
#include <memhook/mapped_storage.h>
#include <memhook/mapped_storage_creator.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/move/unique_ptr.hpp>

namespace memhook
{

class MemDBSession : public enable_shared_from_this<MemDBSession>, private noncopyable
{
public:
    explicit MemDBSession(const shared_ptr<MappedStorageCreator> &storage_creator,
        boost::asio::io_service& io_service);

    void Start();

    boost::asio::ip::tcp::socket& GetSocket()
    {
        return socket_;
    }

private:
    struct ParsingInfo
    {
        std::size_t size;
        ParsingInfo (MemDBSession::*callback)();
        ParsingInfo(std::size_t size, ParsingInfo (MemDBSession::*callback)())
            : size(size), callback(callback)
        {}
    };

    typedef boost::function<ParsingInfo ()> ParsingCallback;

    ParsingInfo ParseOutbound();
    ParsingInfo ParseInbound();

    void OnReadComplete(const ParsingCallback &callback, const boost::system::error_code& e);
    bool HandleNetRequest(NetRequest &request);
    void OnInsert(const TraceInfoBase &traceinfo, const CallStackInfo &callstack);
    void OnErase(const TraceInfoBase &traceinfo);
    void OnUpdateSize(const TraceInfoBase &traceinfo);

    boost::asio::ip::tcp::socket     socket_;
    boost::asio::streambuf           sbuf_;

    shared_ptr<MappedStorageCreator> storage_creator_;
    unique_ptr<MappedStorage>        storage_;
};

} // namespace

#endif
