#include "network_mapped_storage.h"
#include <memhook/serialization.h>
#include <boost/lexical_cast.hpp>

namespace memhook {
  NetworkMappedStorage::NetworkMappedStorage(const char *host, int port)
      : m_host(host)
      , m_port(port)
      , m_iostream()
      , m_sbuf()
      , m_sbuf_mutex() {
    // m_iostream.expires_from_now(chrono::seconds(60));
    m_iostream.connect(host, boost::lexical_cast<std::string>(port));
    if (!m_iostream)
      boost::asio::detail::throw_error(m_iostream.error());
    NetRequest request(kNetReqNewStorage,
            static_cast<uint32_t>(::getpid()), 0, chrono::system_clock::now());
    Send(request);
  }

  NetworkMappedStorage::~NetworkMappedStorage() {}

  void NetworkMappedStorage::Add(uintptr_t address, std::size_t memsize,
          const CallStackInfo &callstack,
          const chrono::system_clock::time_point &timestamp) {
    NetRequest request(kNetReqAdd, address, memsize, timestamp, callstack);
    Send(request);
  }

  bool NetworkMappedStorage::Remove(uintptr_t address) {
    NetRequest request(kNetReqRemove, address);
    Send(request);
    return true;
  }

  bool NetworkMappedStorage::UpdateSize(uintptr_t address, std::size_t memsize) {
    NetRequest request(kNetReqUpdateSize, address, memsize);
    Send(request);
    return true;
  }

  void NetworkMappedStorage::Clear() {
    NetRequest request(kNetReqClear);
    Send(request);
  }

  void NetworkMappedStorage::Flush() {
    m_iostream.flush();
  }

  std::string NetworkMappedStorage::GetName() const {
    return m_host + ':' + boost::lexical_cast<std::string>(m_port);
  }

  void NetworkMappedStorage::Send(const NetRequest &request) {
    NetProtoOutbound outbound(serialization::GetSize(request));
    const std::size_t buf_size = outbound.size + serialization::GetSize(outbound);
    boost::unique_lock<boost::mutex> lock(m_sbuf_mutex);
    boost::asio::mutable_buffer buf = boost::asio::buffer(m_sbuf.prepare(buf_size));
    boost::asio::mutable_buffer tmpbuf = buf;
    serialization::Write(tmpbuf, outbound);
    serialization::Write(tmpbuf, request);
    BOOST_ASSERT(buffer_size(tmpbuf) == 0);
    m_iostream.rdbuf()->sputn(boost::asio::buffer_cast<const char *>(buf), buf_size);
    // m_iostream.flush();
    m_sbuf.consume(buf_size);
  }

  unique_ptr<MappedStorage> NewNetworkMappedStorage(const char *host, int port) {
    return unique_ptr<MappedStorage>(new NetworkMappedStorage(host, port));
  }

  NetworkMappedStorageCreator::NetworkMappedStorageCreator(const char *host, int port)
      : m_host(host), m_port(port) {}

  unique_ptr<MappedStorage> NetworkMappedStorageCreator::New(uintptr_t) const {
    return unique_ptr<MappedStorage>(new NetworkMappedStorage(m_host.c_str(), m_port));
  }

  unique_ptr<MappedStorageCreator> NewNetworkMappedStorageCreator(const char *host, int port) {
    return unique_ptr<MappedStorageCreator>(new NetworkMappedStorageCreator(host, port));
  }
} // namespace
