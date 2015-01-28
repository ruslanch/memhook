#include "memdb_server.hpp"
#include "memdb_session.hpp"

namespace memhook {

memdb_server::memdb_server(asio::io_service& io_service, const std::string &address,
            const std::string &service)
        : io_service_(io_service)
        , acceptor_(io_service) {
    using namespace asio::ip;
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query(address, service);
    tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    start_accept();
}

void memdb_server::start_accept() {
    shared_ptr<memdb_session> new_session(new memdb_session(io_service_));
    acceptor_.async_accept(new_session->get_socket(), bind(&memdb_server::handle_accept,
            this, new_session, asio::placeholders::error));
}

void memdb_server::handle_accept(const shared_ptr<memdb_session> &session,
        const system::error_code& error) {
    if (error)
        std::cerr << error << std::endl;
    else
        session->start();

    start_accept();
}

} // memhook
