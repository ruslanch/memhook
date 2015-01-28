#include "network_storage.hpp"
#include <memhook/network.ipp>
#include <boost/lexical_cast.hpp>

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
    send(req);
}

bool network_storage::erase(uintptr_t address) {
    callstack_container callstack;
    net_req req(net_req_erase, address, 0, system_clock_now(), callstack);
    send(req);
    return true;
}

bool network_storage::update_size(uintptr_t address, std::size_t memsize) {
    callstack_container callstack;
    net_req req(net_req_upd_size, address, memsize, system_clock_now(), callstack);
    send(req);
    return true;
}

void network_storage::send(const net_req &req) {
    const std::size_t size = size_of(req);
    net_proto_outbound outbound(size);
    const std::size_t prepare_size = size + size_of(outbound);
    unique_lock<mutex> lock(ios_mutex_);
    asio::mutable_buffer buf = asio::buffer(buf_.prepare(prepare_size));
    asio::mutable_buffer tmpbuf = buf;
    write(tmpbuf, outbound);
    write(tmpbuf, req);
    ios_.rdbuf()->sputn(asio::buffer_cast<const char *>(buf), prepare_size);
    buf_.consume(prepare_size);
    ios_.flush();
}

} // namespace
