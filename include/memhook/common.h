#ifndef MEMHOOK_INCLUDE_COMMON_H_INCLUDED
#define MEMHOOK_INCLUDE_COMMON_H_INCLUDED

#include <memhook/config.h>

#ifdef BOOST_GCC
#  pragma GCC push_options
#  pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include <boost/noncopyable.hpp>
#include <boost/call_traits.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/chrono/system_clocks.hpp>

#ifdef BOOST_GCC
#  pragma GCC pop_options
#endif

#define MEMHOOK_SHARED_CONTAINER "ShmContainer"
#define MEMHOOK_SHARED_SIMPLE_CONTAINER "ShmSimpleContainer"
#define MEMHOOK_SHARED_MEMORY    "ShmMemHook"
#define MEMHOOK_NETWORK_STORAGE_PORT 20015

namespace memhook {
    using boost::noncopyable;
    using boost::call_traits;
    using boost::movelib::unique_ptr;
    using boost::movelib::make_unique;
    using boost::shared_ptr;
    using boost::make_shared;
    using boost::enable_shared_from_this;

    namespace chrono = boost::chrono;
} // memhook

#endif
