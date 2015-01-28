#include "memdb_session.hpp"
#include <memhook/network.ipp>
#include <boost/bind.hpp>

namespace memhook {

memdb_session::memdb_session(asio::io_service& io_service)
        : sock_(io_service)
        , sbuf_() {}

void memdb_session::start() {
    const std::size_t outbound_size = sizeof(net_proto_outbound);
    asio::async_read(sock_, sbuf_, asio::transfer_exactly(outbound_size),
        bind(&memdb_session::read_outbound_complete, shared_from_this(), asio::placeholders::error)
    );
}

void memdb_session::read_outbound_complete(const system::error_code& e) {
    if (e) {
        std::cout << e.message() << std::endl;
        return;
    }

    asio::streambuf::const_buffers_type bufs = sbuf_.data();
    asio::const_buffer buf = asio::buffer(bufs);
    const std::size_t  buf_size = asio::buffer_size(buf);

    net_proto_outbound outbound;
    read(buf, outbound);
    sbuf_.consume(buf_size - asio::buffer_size(buf));

    asio::async_read(sock_, sbuf_, asio::transfer_exactly(outbound.size),
        bind(&memdb_session::read_inbound_complete, shared_from_this(), asio::placeholders::error)
    );
}

void memdb_session::read_inbound_complete(const system::error_code& e) {
    if (e) {
        std::cout << e.message() << std::endl;
        return;
    }

    asio::streambuf::const_buffers_type bufs = sbuf_.data();
    asio::const_buffer buf = asio::buffer(bufs);
    const std::size_t  buf_size = asio::buffer_size(buf);

    net_req req;
    read(buf, req);
    sbuf_.consume(buf_size - asio::buffer_size(buf));

    start();
}

} // memhook
