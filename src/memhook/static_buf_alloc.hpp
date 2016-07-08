#ifndef MEMHOOK_STATIC_BUF_ALLOC_HPP_INCLUDED
#define MEMHOOK_STATIC_BUF_ALLOC_HPP_INCLUDED

#include <memhook/common.hpp>
#include <cstddef>

namespace memhook {

/* simple allocator used when the program starts */
class StaticBufAlloc {
public:
    static void* malloc(size_t size);
    static void* calloc(size_t nmemb, size_t size);
    static void  free(void *ptr);

private:
    static char   tmpbuf_[1024];
    static size_t tmppos_;
};

} // namespace memhook

#endif
