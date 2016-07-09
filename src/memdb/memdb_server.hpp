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

class memdb_server : private boost::noncopyable {
public:
    memdb_server(const boost::shared_ptr<mapped_storage> &storage,
            boost::asio::io_service& io_service,
            const char *host, int port);

private:
    void start_accept();
    void handle_accept(const boost::shared_ptr<memdb_session> &session,
            const boost::system::error_code& error);

    boost::asio::io_service&          io_service_;
    boost::asio::ip::tcp::acceptor    acceptor_;
    boost::shared_ptr<mapped_storage> storage_;
};

} // namespace

#endif // MEMHOOK_MEMDB_SERVER_HPP_INCLUDED
