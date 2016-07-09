#ifndef MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED
#define MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/network.hpp>
#include <memhook/mapped_storage.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

namespace memhook {

class network_storage : public mapped_storage {
public:
    network_storage(const char *host, int port);
    void insert(uintptr_t address, std::size_t memsize,
        callstack_container &callstack);
    void insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        callstack_container &callstack);
    bool erase(uintptr_t address);
    bool update_size(uintptr_t address, std::size_t memsize);

private:
    void send(const net_request &request);

    boost::asio::ip::tcp::iostream iostream_;
    boost::asio::streambuf         sbuf_;
    boost::mutex                   sbuf_mutex_;
};

mapped_storage *make_network_storage(const char *host, int port);

} // memhook

#endif // MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED
