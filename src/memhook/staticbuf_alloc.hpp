#ifndef MEMHOOK_STATICBUF_ALLOC_HPP_INCLUDED
#define MEMHOOK_STATICBUF_ALLOC_HPP_INCLUDED

namespace memhook {

/* simple allocator used when the program starts */
class StaticBufAlloc {
public:
    static void* malloc(size_t size) BOOST_NOEXCEPT_OR_NOTHROW;
    static void* calloc(size_t nmemb, size_t size) BOOST_NOEXCEPT_OR_NOTHROW;
    static void  free(void *ptr) BOOST_NOEXCEPT_OR_NOTHROW;

private:
    static char        tmpbuf_[1024];
    static std::size_t tmppos_;
};

} // namespace memhook

#endif
