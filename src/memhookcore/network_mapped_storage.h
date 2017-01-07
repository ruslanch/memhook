#ifndef MEMHOOK_SRC_MEMSTORAGE_NETWORK_MAPPED_STORAGE_H_INCLUDED
#define MEMHOOK_SRC_MEMSTORAGE_NETWORK_MAPPED_STORAGE_H_INCLUDED

#include "common.h"

#include <memhook/mapped_storage.h>
#include <memhook/mapped_storage_creator.h>
#include <memhook/network.h>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace memhook {
  class NetworkMappedStorage : public MappedStorage {
  public:
    NetworkMappedStorage(const char *host, int port);
    ~NetworkMappedStorage();

    void Add(uintptr_t address, std::size_t memsize,
            const CallStackInfo &callstack,
            const chrono::system_clock::time_point &timestamp);
    bool Remove(uintptr_t address);
    bool UpdateSize(uintptr_t address, std::size_t memsize);
    void Clear();
    void Flush();
    std::string GetName() const;

  private:
    void Send(const NetRequest &request);

    std::string m_host;
    int         m_port;

    boost::asio::ip::tcp::iostream m_iostream;
    boost::asio::streambuf         m_sbuf;
    boost::mutex                   m_sbuf_mutex;
  };

  class NetworkMappedStorageCreator : public MappedStorageCreator {
  public:
      NetworkMappedStorageCreator(const char *host, int port);
      unique_ptr<MappedStorage> New(uintptr_t guide) const;

  private:
      std::string m_host;
      int m_port;
  };
}

#endif
