#ifndef MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED
#define MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/network.hpp>
#include "mapped_storage.hpp"
#include <boost/asio.hpp>

namespace memhook {

class network_storage : public mapped_storage {
public:
    network_storage(const char *host, int port);
    void insert(uintptr_t address, std::size_t memsize, callstack_container &callstack);
    bool erase(uintptr_t address);
    bool update_size(uintptr_t address, std::size_t memsize);

private:
    asio::ip::tcp::iostream ios_;
};

} // memhook

#endif // MEMHOOK_NETWORK_STORAGE_HPP_INCLUDED
