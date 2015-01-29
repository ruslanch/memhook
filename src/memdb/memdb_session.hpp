#ifndef MEMHOOK_MEMDB_SESSION_HPP_INCLUDED
#define MEMHOOK_MEMDB_SESSION_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/network.hpp>
#include <memhook/mapped_storage.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
// #include <boost/.hpp>

namespace memhook {

class memdb_session : public enable_shared_from_this<memdb_session>, private noncopyable {
public:
    explicit memdb_session(const shared_ptr<mapped_storage> &storage, asio::io_service& io_service);
    void start();

    asio::ip::tcp::socket& get_socket() { return socket_; }

private:
    struct parsing_info {
        std::size_t size;
        parsing_info (memdb_session::*callback)();
        parsing_info(std::size_t size, parsing_info (memdb_session::*callback)())
            : size(size), callback(callback) {}
    };
    typedef function<parsing_info ()> parsing_callback;

    parsing_info parse_outbound();
    parsing_info parse_inbound();

    void on_read_complete(const parsing_callback &callback, const system::error_code& e);
    bool do_handle(net_request &request);
    void do_insert(const traceinfo_base &traceinfo, callstack_container &callstack);
    void do_erase(const traceinfo_base &traceinfo);
    void do_upd_size(const traceinfo_base &traceinfo);

    asio::ip::tcp::socket socket_;
    asio::streambuf       sbuf_;
    shared_ptr<mapped_storage> storage_;
};

} // namespace

#endif // MEMHOOK_MEMDB_SESSION_HPP_INCLUDED
