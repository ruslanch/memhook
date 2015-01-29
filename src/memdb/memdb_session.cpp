#include "memdb_session.hpp"
#include <memhook/network.ipp>
#include <boost/bind.hpp>

namespace memhook {

memdb_session::memdb_session(const shared_ptr<mapped_storage> &storage, asio::io_service& io_service)
        : socket_(io_service)
        , sbuf_()
        , storage_(storage) {}

void memdb_session::start() {
    const std::size_t outbound_size = sizeof(net_proto_outbound);
    asio::async_read(socket_, sbuf_, asio::transfer_at_least(outbound_size),
        bind(&memdb_session::on_read_complete, shared_from_this(),
                parsing_callback(bind(&memdb_session::parse_outbound, this)),
                asio::placeholders::error)
    );
}

void memdb_session::on_read_complete(const parsing_callback &callback,
        const system::error_code& e) {
    if (e) {
        std::cout << e.message() << std::endl;
        return;
    }

    parsing_info parsing_info = callback();
    if (parsing_info.size == 0) {
        std::cout << "parsing_info.size=0, close connection" << std::endl;
        return;
    }

    asio::async_read(socket_, sbuf_, asio::transfer_exactly(parsing_info.size),
        bind(&memdb_session::on_read_complete, shared_from_this(),
                parsing_callback(bind(parsing_info.callback, this)),
                asio::placeholders::error)
    );
}

memdb_session::parsing_info memdb_session::parse_outbound() {
    asio::const_buffer inputbuf = asio::buffer(sbuf_.data());
    std::size_t  bytes_inputbuf = asio::buffer_size(inputbuf);

    net_proto_outbound outbound;
    read(inputbuf, outbound);

    std::size_t bytes_read = bytes_inputbuf - asio::buffer_size(inputbuf);
    sbuf_.consume(bytes_read);
    bytes_inputbuf -= bytes_read;
    if (bytes_inputbuf < outbound.size)
        return parsing_info(outbound.size, &memdb_session::parse_inbound);
    return parse_inbound();
}

memdb_session::parsing_info memdb_session::parse_inbound() {
    asio::const_buffer inputbuf = asio::buffer(sbuf_.data());
    std::size_t  bytes_inputbuf = asio::buffer_size(inputbuf);

    net_request request;
    read(inputbuf, request);
    if (!do_handle(request))
        memdb_session::parsing_info(0, NULL);

    std::size_t bytes_read = bytes_inputbuf - asio::buffer_size(inputbuf);
    sbuf_.consume(bytes_read);
    bytes_inputbuf -= bytes_read;

    if (bytes_inputbuf < sizeof(net_proto_outbound))
        return parsing_info(sizeof(net_proto_outbound), &memdb_session::parse_outbound);
    return parse_outbound();
}

bool memdb_session::do_handle(net_request &request) {
    switch (request.type) {
        case net_req_insert:
            do_insert(request.traceinfo, request.callstack);
            return true;
        case net_req_erase:
            do_erase(request.traceinfo);
            return true;
        case net_req_upd_size:
            do_upd_size(request.traceinfo);
            return true;
        case net_req_fetch:
        case net_req_fetch_end:
        case net_req_end:
            break;
    }
    return false;
}

void memdb_session::do_insert(const traceinfo_base &traceinfo,
        callstack_container &callstack) {
    storage_->insert(traceinfo.address, traceinfo.memsize, traceinfo.timestamp, callstack);
}

void memdb_session::do_erase(const traceinfo_base &traceinfo) {
    storage_->erase(traceinfo.address);
}

void memdb_session::do_upd_size(const traceinfo_base &traceinfo) {
    storage_->update_size(traceinfo.address, traceinfo.memsize);
}

} // memhook
