#include "common.h"

#include <memhook/network.h>
#include <memhook/serialization.h>
#include <memhook/mapped_storage.h>
#include <memhook/mapped_storage_creator.h>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/move/unique_ptr.hpp>

namespace memhook {

class NetworkMappedStorage : public MappedStorage
{
public:
    NetworkMappedStorage(const char *host, int port);
    ~NetworkMappedStorage();
    void Insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        const CallStackInfo &callstack);
    bool Erase(uintptr_t address);
    bool UpdateSize(uintptr_t address, std::size_t memsize);
    void Clear();
    void Flush();
    std::string GetName() const;

private:
    void Send(const NetRequest &request);

    std::string host_;
    int         port_;

    boost::asio::ip::tcp::iostream iostream_;
    boost::asio::streambuf         sbuf_;
    boost::mutex                   sbuf_mutex_;
};

NetworkMappedStorage::NetworkMappedStorage(const char *host, int port)
        : host_(host)
        , port_(port)
        , iostream_()
        , sbuf_()
        , sbuf_mutex_()
{
    // iostream_.expires_from_now(chrono::seconds(60));
    iostream_.connect(host, boost::lexical_cast<std::string>(port));
    if (!iostream_)
        boost::asio::detail::throw_error(iostream_.error());

    NetRequest request(NetReqNewStorage, static_cast<uint32_t>(::getpid()), 0,
        boost::chrono::system_clock::now());
    Send(request);
}

NetworkMappedStorage::~NetworkMappedStorage()
{}

void NetworkMappedStorage::Insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        const CallStackInfo &callstack)
{
    NetRequest request(NetReqInsert, address, memsize, timestamp, callstack);
    Send(request);
}

bool NetworkMappedStorage::Erase(uintptr_t address)
{
    NetRequest request(NetReqErase, address, 0, boost::chrono::system_clock::now());
    Send(request);
    return true;
}

bool NetworkMappedStorage::UpdateSize(uintptr_t address, std::size_t memsize)
{
    CallStackInfo callstack;
    NetRequest request(NetReqUpdateSize, address, memsize, boost::chrono::system_clock::now(),
        callstack);
    Send(request);
    return true;
}

void NetworkMappedStorage::Clear()
{
    NetRequest request(NetReqClear, 0, 0, boost::chrono::system_clock::now());
    Send(request);
}

void NetworkMappedStorage::Flush()
{
    iostream_.flush();
}

std::string NetworkMappedStorage::GetName() const
{
    return host_ + ':' + boost::lexical_cast<std::string>(port_);
}

void NetworkMappedStorage::Send(const NetRequest &request)
{
    NetProtoOutbound outbound(serialization::GetSize(request));
    const std::size_t buf_size = outbound.size + serialization::GetSize(outbound);

    boost::unique_lock<boost::mutex> lock(sbuf_mutex_);
    boost::asio::mutable_buffer buf = boost::asio::buffer(sbuf_.prepare(buf_size));
    boost::asio::mutable_buffer tmpbuf = buf;
    serialization::Write(tmpbuf, outbound);
    serialization::Write(tmpbuf, request);
    BOOST_ASSERT(buffer_size(tmpbuf) == 0);
    iostream_.rdbuf()->sputn(boost::asio::buffer_cast<const char *>(buf), buf_size);
    // iostream_.flush();
    sbuf_.consume(buf_size);
}

unique_ptr<MappedStorage> NewNetworkMappedStorage(const char *host, int port)
{
    return unique_ptr<MappedStorage>(new NetworkMappedStorage(host, port));
}

class NetworkMappedStorageCreator : public MappedStorageCreator
{
public:
    NetworkMappedStorageCreator(const char *host, int port);
    unique_ptr<MappedStorage> New(uintptr_t guide) const;

private:
    std::string host_;
    int port_;
};

NetworkMappedStorageCreator::NetworkMappedStorageCreator(const char *host, int port)
        : host_(host), port_(port)
{}

unique_ptr<MappedStorage> NetworkMappedStorageCreator::New(uintptr_t) const
{
    return unique_ptr<MappedStorage>(new NetworkMappedStorage(host_.c_str(), port_));
}

unique_ptr<MappedStorageCreator> NewNetworkMappedStorageCreator(const char *host, int port)
{
    return unique_ptr<MappedStorageCreator>(new NetworkMappedStorageCreator(host, port));
}

} // namespace
