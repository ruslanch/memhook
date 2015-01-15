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
    typedef chrono::system_clock system_clock_t;
    typedef container::string          string_t;
} // memhook

#endif // MEMHOOK_COMMON_HPP_INCLUDED
