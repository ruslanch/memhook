#ifndef MEMHOOK_MEMDB_SESSION_HPP_INCLUDED
#define MEMHOOK_MEMDB_SESSION_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/network.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

namespace memhook {

class memdb_session : public enable_shared_from_this<memdb_session>, private noncopyable {
public:
    explicit memdb_session(asio::io_service& io_service);
    void start();

    asio::ip::tcp::socket& get_socket() { return sock_; }

private:
    void read_outbound_complete(const system::error_code& e);
    void read_inbound_complete(const system::error_code& e);

    asio::ip::tcp::socket sock_;
    asio::streambuf       sbuf_;
};

} // namespace

#endif // MEMHOOK_MEMDB_SESSION_HPP_INCLUDED
