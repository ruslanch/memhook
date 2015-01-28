#ifndef MEMHOOK_MEMDB_SERVER_HPP_INCLUDED
#define MEMHOOK_MEMDB_SERVER_HPP_INCLUDED

#include <memhook/common.hpp>
#include "memdb_session.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

namespace memhook {

class memdb_session;

class memdb_server : private noncopyable {
public:
    memdb_server(asio::io_service& io_service, const std::string &address,
        const std::string &service);

private:
    void start_accept();
    void handle_accept(const shared_ptr<memdb_session> &session, const system::error_code& error);

    asio::io_service&       io_service_;
    asio::ip::tcp::acceptor acceptor_;
};

} // namespace

#endif // MEMHOOK_MEMDB_SERVER_HPP_INCLUDED
