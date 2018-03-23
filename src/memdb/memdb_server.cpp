#include "memdb_server.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

namespace memhook
{

MemDBServer::MemDBServer(const shared_ptr<MappedStorageCreator> &storage_creator,
            boost::asio::io_service& io_service,
            const char *host, int port)
        : io_service_(io_service)
        , acceptor_(io_service)
        , storage_creator_(storage_creator)
{
    using namespace boost::asio::ip;
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
    tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    StartAccept();
}

void MemDBServer::StartAccept()
{
    boost::shared_ptr<MemDBSession> new_session(new MemDBSession(storage_creator_, io_service_));
    acceptor_.async_accept(new_session->GetSocket(), bind(&MemDBServer::HandleAccept,
            this, new_session, boost::asio::placeholders::error));
}

void MemDBServer::HandleAccept(const boost::shared_ptr<MemDBSession> &session,
        const boost::system::error_code& error)
{
    if (error)
        std::cerr << error << std::endl;
    else
        session->Start();

    StartAccept();
}

} // memhook
