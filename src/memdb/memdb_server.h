#ifndef MEMHOOK_SRC_MEMDB_SERVER_H_INCLUDED
#define MEMHOOK_SRC_MEMDB_SERVER_H_INCLUDED

#include "common.h"
#include "memdb_session.h"

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/move/core.hpp>

namespace memhook {

class MemDBSession;

class MemDBServer : private noncopyable {
public:
    MemDBServer(const shared_ptr<MappedStorageCreator> &storage_creator,
            boost::asio::io_service& io_service,
            const char *host, int port);

private:
    void StartAccept();
    void HandleAccept(const boost::shared_ptr<MemDBSession> &session,
            const boost::system::error_code& error);

    boost::asio::io_service&         io_service_;
    boost::asio::ip::tcp::acceptor   acceptor_;
    shared_ptr<MappedStorageCreator> storage_creator_;
};

} // namespace

#endif
