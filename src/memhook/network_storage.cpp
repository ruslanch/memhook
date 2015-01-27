#include "network_storage.hpp"
#include <boost/lexical_cast.hpp>

#include "memhook/network.inc"

namespace memhook {

mapped_storage *make_net_storage(const char *host, int port) {
    return new network_storage(host, port);
}

network_storage::network_storage(const char *host, int port)
        : ios_() {
    ios_.expires_from_now(boost::posix_time::seconds(60));
    ios_.connect(host, lexical_cast<std::string>(port));
    if (!ios_)
        asio::detail::throw_error(ios_.error());
}

void network_storage::insert(uintptr_t address, std::size_t memsize,
        callstack_container &callstack) {
    net_req req(net_req_insert, address, memsize, system_clock_now(), callstack);
    write(ios_.rdbuf(), req);
    ios_.flush();
}

bool network_storage::erase(uintptr_t address) {
    callstack_container callstack;
    net_req req(net_req_erase, address, 0, system_clock_now(), callstack);
    write(ios_.rdbuf(), req);
    ios_.flush();
    return true;
}

bool network_storage::update_size(uintptr_t address, std::size_t memsize) {
    callstack_container callstack;
    net_req req(net_req_upd_size, address, memsize, system_clock_now(), callstack);
    write(ios_.rdbuf(), req);
    ios_.flush();
    return true;
}

} // namespace
