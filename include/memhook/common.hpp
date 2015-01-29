#ifndef MEMHOOK_COMMON_HPP_INCLUDED
#define MEMHOOK_COMMON_HPP_INCLUDED

#include "config.hpp"
#include <boost/cstdint.hpp>
#include <boost/chrono/system_clocks.hpp>

#ifndef BOOST_ASIO_DISABLE_BOOST_DATE_TIME
#   define BOOST_ASIO_DISABLE_BOOST_DATE_TIME 1
#endif

#define MEMHOOK_SHARED_CONTAINER "ShmContainer"
#define MEMHOOK_SHARED_MEMORY    "ShmMemHook"
#define MEMHOOK_NETWORK_STORAGE_PORT 20015

namespace memhook {
    using chrono::system_clock;
} // memhook

#endif // MEMHOOK_COMMON_HPP_INCLUDED
