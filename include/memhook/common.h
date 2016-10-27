#ifndef MEMHOOK_INCLUDE_COMMON_H_INCLUDED
#define MEMHOOK_INCLUDE_COMMON_H_INCLUDED

#include <memhook/config.h>

#include <boost/noncopyable.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

#define MEMHOOK_SHARED_CONTAINER "ShmContainer"
#define MEMHOOK_SHARED_MEMORY    "ShmMemHook"
#define MEMHOOK_NETWORK_STORAGE_PORT 20015

namespace memhook
{
    using boost::noncopyable;
    using boost::movelib::unique_ptr;
    using boost::movelib::make_unique;
    using boost::shared_ptr;
    using boost::make_shared;
    using boost::enable_shared_from_this;
} // memhook

#endif
