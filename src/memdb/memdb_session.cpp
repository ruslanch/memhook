#include "memdb_session.h"

#include <memhook/serialization.h>

#include <boost/bind.hpp>

namespace memhook
{

MemDBSession::MemDBSession(const shared_ptr<MappedStorageCreator> &storage_creator,
            boost::asio::io_service& io_service)
    : socket_(io_service)
    , sbuf_()
    , storage_creator_(storage_creator)
    , storage_()
{}

void MemDBSession::Start()
{
    const std::size_t outbound_size = sizeof(NetProtoOutbound);
    boost::asio::async_read(socket_, sbuf_, boost::asio::transfer_at_least(outbound_size),
        boost::bind(&MemDBSession::OnReadComplete, shared_from_this(),
                ParsingCallback(boost::bind(&MemDBSession::ParseOutbound, this)),
                boost::asio::placeholders::error)
    );
}

void MemDBSession::OnReadComplete(const ParsingCallback &callback,
        const boost::system::error_code& e)
{
    if (e)
    {
        if (e == boost::asio::error::eof)
        {
            if (storage_)
                std::cout << "Close storage, " << storage_->GetName() << std::endl;
            else
                std::cout << "Close connection" << std::endl;
        }
        else
        {
            std::cout << e.message() << "" << std::endl;
        }

        return;
    }

    ParsingInfo parsing_info = callback();
    if (parsing_info.size == 0)
    {
        std::cout << "parsing_info.size=0, close connection" << std::endl;
        return;
    }

    boost::asio::async_read(socket_, sbuf_, boost::asio::transfer_exactly(parsing_info.size),
        boost::bind(&MemDBSession::OnReadComplete, shared_from_this(),
                ParsingCallback(boost::bind(parsing_info.callback, this)),
                boost::asio::placeholders::error)
    );
}

MemDBSession::ParsingInfo MemDBSession::ParseOutbound()
{
    boost::asio::const_buffer inputbuf = boost::asio::buffer(sbuf_.data());
    std::size_t  bytes_inputbuf = boost::asio::buffer_size(inputbuf);

    if (bytes_inputbuf < sizeof(NetProtoOutbound))
    {
        std::cout << "Not enough data, something went wrong" << std::endl;
        return ParsingInfo(sizeof(NetProtoOutbound) - bytes_inputbuf, &MemDBSession::ParseOutbound);
    }

    NetProtoOutbound outbound;
    serialization::Read(inputbuf, outbound);

    const std::size_t bytes_read = bytes_inputbuf - boost::asio::buffer_size(inputbuf);
    bytes_inputbuf -= bytes_read;
    sbuf_.consume(bytes_read);

    if (bytes_inputbuf < outbound.size)
        return ParsingInfo(outbound.size - bytes_inputbuf, &MemDBSession::ParseOutbound);

    return ParseInbound();
}

MemDBSession::ParsingInfo MemDBSession::ParseInbound()
{
    boost::asio::const_buffer inputbuf = boost::asio::buffer(sbuf_.data());
    std::size_t  bytes_inputbuf = boost::asio::buffer_size(inputbuf);

    NetRequest request;
    serialization::Read(inputbuf, request);
    if (!HandleNetRequest(request))
        return MemDBSession::ParsingInfo(0, NULL);

    const std::size_t bytes_read = bytes_inputbuf - boost::asio::buffer_size(inputbuf);
    bytes_inputbuf -= bytes_read;
    sbuf_.consume(bytes_read);

    if (bytes_inputbuf < sizeof(NetProtoOutbound))
        return ParsingInfo(sizeof(NetProtoOutbound) - bytes_inputbuf, &MemDBSession::ParseOutbound);

    return ParseOutbound();
}

bool MemDBSession::HandleNetRequest(NetRequest &request)
{
    switch (request.type)
    {
        case NetReqInsert:
            OnInsert(request.traceinfo, request.callstack);
            return true;
        case NetReqErase:
            OnErase(request.traceinfo);
            return true;
        case NetReqUpdateSize:
            OnUpdateSize(request.traceinfo);
            return true;
        case NetReqNewStorage:
            OnNewStorage(request.traceinfo.address);
            return true;
        case NetReqUnknown:
        case NetReqFetch:
        case NetReqFetchEnd:
        case NetReqEnd:
            break;
    }
    return false;
}

void MemDBSession::OnInsert(const TraceInfoBase &traceinfo, const CallStackInfo &callstack)
{
    storage_->Insert(traceinfo.address, traceinfo.memsize, traceinfo.timestamp, callstack);
}

void MemDBSession::OnErase(const TraceInfoBase &traceinfo)
{
    storage_->Erase(traceinfo.address);
}

void MemDBSession::OnUpdateSize(const TraceInfoBase &traceinfo)
{
    storage_->UpdateSize(traceinfo.address, traceinfo.memsize);
}

void MemDBSession::OnNewStorage(uintptr_t pid)
{
    storage_ = storage_creator_->New(pid);
    std::cout << "Open storage, " << storage_->GetName() << std::endl;
}

} // memhook
