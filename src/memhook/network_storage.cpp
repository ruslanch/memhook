#include "network_storage.hpp"
#include <memhook/network.ipp>
#include <boost/lexical_cast.hpp>

namespace memhook {

network_storage::network_storage(const char *host, int port)
        : iostream_()
        , sbuf_()
        , sbuf_mutex_() {
    iostream_.expires_from_now(chrono::seconds(60));
    iostream_.connect(host, lexical_cast<std::string>(port));
    if (!iostream_)
        asio::detail::throw_error(iostream_.error());
}

void network_storage::insert(uintptr_t address, std::size_t memsize,
        callstack_container &callstack) {
    insert(address, memsize, system_clock::now(), callstack);
}

void network_storage::insert(uintptr_t address, std::size_t memsize,
        const system_clock::time_point &timestamp,
        callstack_container &callstack) {
    net_request request(net_req_insert, address, memsize, timestamp, callstack);
    send(request);
}

bool network_storage::erase(uintptr_t address) {
    callstack_container callstack;
    net_request request(net_req_erase, address, 0, system_clock::now(), callstack);
    send(request);
    return true;
}

bool network_storage::update_size(uintptr_t address, std::size_t memsize) {
    callstack_container callstack;
    net_request request(net_req_upd_size, address, memsize, system_clock::now(), callstack);
    send(request);
    return true;
}

void network_storage::send(const net_request &request) {
    net_proto_outbound outbound(size_of(request));
    const std::size_t buf_size = outbound.size + size_of(outbound);

    unique_lock<mutex> lock(sbuf_mutex_);
    asio::mutable_buffer buf = asio::buffer(sbuf_.prepare(buf_size));
    asio::mutable_buffer tmpbuf = buf;
    write(tmpbuf, outbound);
    write(tmpbuf, request);
    BOOST_ASSERT(buffer_size(tmpbuf) == 0);

    iostream_.rdbuf()->sputn(asio::buffer_cast<const char *>(buf), buf_size);
    iostream_.flush();
    sbuf_.consume(buf_size);
}

mapped_storage *make_network_storage(const char *host, int port) {
    return new network_storage(host, port);
}

} // namespace
