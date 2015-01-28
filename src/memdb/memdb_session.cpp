#include "memdb_session.hpp"
#include <memhook/network.inc>
#include <boost/bind.hpp>

namespace memhook {

memdb_session::memdb_session(asio::io_service& io_service)
        : sock_(io_service)
        , sbuf_() {}

void memdb_session::start() {
    asio::async_read(sock_, sbuf_, asio::transfer_exactly(sizeof(net_proto_outbound)),
        bind(&memdb_session::read_outbound_complete, shared_from_this(), asio::placeholders::error)
    );
}

void memdb_session::read_outbound_complete(const system::error_code& e) {
    if (e) {
        std::cout << e.message() << std::endl;
        return;
    }

    net_proto_outbound outbound;
    read(&sbuf_, outbound);

    // std::cout << "outbound.size=" << outbound.size << std::endl;

    asio::async_read(sock_, sbuf_, asio::transfer_exactly(outbound.size),
        bind(&memdb_session::read_inbound_complete, shared_from_this(), asio::placeholders::error)
    );
}

void memdb_session::read_inbound_complete(const system::error_code& e) {
    if (e) {
        std::cout << e.message() << std::endl;
        return;
    }

    net_req req;
    read(&sbuf_, req);

    // std::cout << std::hex << std::showbase << req.type
    //     << " " << req.traceinfo.address
    //     << " " << std::dec << req.traceinfo.memsize
    //     << std::endl;

    start();
}

} // memhook
