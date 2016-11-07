#ifndef MEMHOOK_SRC_MEMSTORAGE_NETWORK_MAPPED_STORAGE_H_INCLUDED
#define MEMHOOK_SRC_MEMSTORAGE_NETWORK_MAPPED_STORAGE_H_INCLUDED

#include "common.h"

#include <memhook/mapped_storage.h>
#include <memhook/network.h>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

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

} // memhook

#endif
