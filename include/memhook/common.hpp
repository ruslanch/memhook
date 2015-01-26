#ifndef MEMHOOK_COMMON_HPP_INCLUDED
#define MEMHOOK_COMMON_HPP_INCLUDED

#include "config.hpp"
#include <boost/cstdint.hpp>
#include <boost/chrono/system_clocks.hpp>

#define MEMHOOK_SHARED_CONTAINER "ShmContainer"
#define MEMHOOK_SHARED_MEMORY    "ShmMemHook"

namespace memhook {
    using chrono::system_clock;

    inline
    system_clock::time_point system_clock_now() BOOST_NOEXCEPT {
        timespec ts = {0};
        clock_gettime(CLOCK_REALTIME, &ts);
        return system_clock::time_point(system_clock::duration(
                static_cast<system_clock::rep>(ts.tv_sec) * 1000000000 + ts.tv_nsec));
    }
} // memhook

#endif // MEMHOOK_COMMON_HPP_INCLUDED
