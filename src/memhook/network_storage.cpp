#include "network_storage.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>

namespace memhook {

mapped_storage *make_net_storage(const char *host, int port) {
    return new network_storage(host, port);
}

network_storage::network_storage(const char *host, int port)
        : iostream_() {
    iostream_.expires_from_now(boost::posix_time::seconds(60));
    iostream_.connect(host, lexical_cast<std::string>(port));
    if (!iostream_)
        asio::detail::throw_error(iostream_.error());
}

void network_storage::insert(uintptr_t address, std::size_t memsize,
        const callstack_container &callstack) {
    net_req req(net_req_insert, address, memsize, system_clock_now(), callstack);

    iostream_.flush();
}

bool network_storage::erase(uintptr_t address) {
    return false;
}

bool network_storage::update_size(uintptr_t address, std::size_t memsize) {
    return false;
}

} // namespace
