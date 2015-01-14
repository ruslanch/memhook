#ifndef MEMHOOK_SCOPED_USE_COUNT_HPP_INCLUDED
#define MEMHOOK_SCOPED_USE_COUNT_HPP_INCLUDED

#include "common.hpp"
#include <boost/noncopyable.hpp>

namespace memhook {

class scoped_use_count : noncopyable {
    ssize_t *n_;
public:
    explicit scoped_use_count(ssize_t *n) : n_(n) {
        __sync_fetch_and_add(n_, 1);
    }

    ~scoped_use_count() {
        __sync_fetch_and_sub(n_, 1);
    }
};

} // memhook

#endif // MEMHOOK_SCOPED_USE_COUNT_HPP_INCLUDED
