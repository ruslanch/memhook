#ifndef MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED
#define MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/network.hpp>
#include "mapped_storage.hpp"
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace memhook {

class network_storage : public mapped_storage {
public:
    network_storage(const char *host, int port);
    void insert(uintptr_t address, std::size_t memsize, callstack_container &callstack);
    bool erase(uintptr_t address);
    bool update_size(uintptr_t address, std::size_t memsize);

private:
    void send(const net_req &req);

    asio::ip::tcp::iostream ios_;
    mutex ios_mutex_;
};

} // memhook

#endif // MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED
