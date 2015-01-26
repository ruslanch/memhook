#ifndef MEMHOOK_COMMON_HPP_INCLUDED
#define MEMHOOK_COMMON_HPP_INCLUDED

#include "config.hpp"
#include <boost/cstdint.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/container/string.hpp>
#include <string>

#define MEMHOOK_SHARED_CONTAINER "ShmContainer"
#define MEMHOOK_SHARED_MEMORY    "ShmMemHook"

namespace memhook {
    typedef chrono::system_clock        system_clock_t;
    typedef system_clock_t::time_point  time_point_t;
    typedef system_clock_t::duration    duration_t;
    typedef duration_t::rep             timestamp_t;
    typedef container::string           string_t;

    inline time_point_t system_clock_now() BOOST_NOEXCEPT {
        timespec ts = {0};
        clock_gettime(CLOCK_REALTIME, &ts);
        return time_point_t(duration_t(static_cast<system_clock_t::rep>(ts.tv_sec) * 1000000000
            + ts.tv_nsec));
    }
} // memhook

#endif // MEMHOOK_COMMON_HPP_INCLUDED
