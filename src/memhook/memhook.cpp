#include "common.h"
#include "no_hook.h"
#include "glibc.h"
#include "log.h"
#include "dl_functions.h"
#include "engine.h"
#include "spin_lock.h"

#ifdef BOOST_GCC
#  pragma GCC push_options
#  pragma GCC optimize("no-optimize-sibling-calls")
#endif

namespace memhook
{
  namespace {
    DLFunctions g_dl_functions(DLFunctions::kLINKER_INITIALIZED);

    struct MemHook_InitHelper {
      MemHook_InitHelper() {
        NoHook no_hook;
        Engine::Initialize();
      }

      ~MemHook_InitHelper() {
        NoHook no_hook;
        Engine::Destroy();
      }
    } g_init_helper;

    const DLFunctions &GetDLFunctions() {
      g_dl_functions.Initialize();
      return g_dl_functions;
    }
  }

  void do_free(void *mem) MEMHOOK_NOEXCEPT {
    GetDLFunctions().free(mem);
  }

  void *do_malloc(size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().malloc(size);
  }

  void *do_calloc(size_t nmemb, size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().calloc(nmemb, size);
  }

  void *do_realloc(void *mem, size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().realloc(mem, size);
  }

  void *do_memalign(size_t alignment, size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().memalign(alignment, size);
  }

  int do_posix_memalign(void **memptr, size_t alignment, size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().posix_memalign(memptr, alignment, size);
  }

  void do_cfree(void *mem) MEMHOOK_NOEXCEPT {
    GetDLFunctions().cfree(mem);
  }

#if (HAVE_ALIGNED_ALLOC+0)
  void *do_aligned_alloc(size_t alignment, size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().aligned_alloc(alignment, size);
  }
#endif

#if (HAVE_VALLOC+0)
  void *do_valloc(size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().valloc(size);
  }
#endif

#if (HAVE_PVALLOC+0)
  void *do_pvalloc(size_t size) MEMHOOK_NOEXCEPT {
    return GetDLFunctions().pvalloc(size);
  }
#endif

  void *do_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
          MEMHOOK_NOEXCEPT {
    return GetDLFunctions().mmap(addr, length, prot, flags, fd, offset);
  }

  void *do_mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset)
          MEMHOOK_NOEXCEPT {
    return GetDLFunctions().mmap64(addr, length, prot, flags, fd, offset);
  }

  void *do_mremap(void *addr, size_t old_len, size_t new_len, int flags, ...)
          MEMHOOK_NOEXCEPT {
    va_list ap;
    va_start(ap, flags);
    void *new_addr_arg = va_arg(ap, void *);
    va_end(ap);
    return GetDLFunctions().mremap(addr, old_len, new_len, flags, new_addr_arg);
  }

  int do_munmap(void *addr, size_t length)
          MEMHOOK_NOEXCEPT {
    return GetDLFunctions().munmap(addr, length);
  }

  static SpinLock g_set_new_handler_lock(SpinLock::kLINKER_INITIALIZED);

  void *do_cpp_alloc2(size_t size, bool nothrow) {
    void *mem = do_malloc(size);
    while (BOOST_UNLIKELY(mem == NULL)) {
      std::new_handler nh;
      {
        SpinLockHolder holder(g_set_new_handler_lock);
        nh = std::set_new_handler(0);
        (void)std::set_new_handler(nh);
      }

      if (!nh) {
        if (!nothrow)
          throw std::bad_alloc();
        return NULL;
      }

      try {
        (*nh)();
      } catch (const std::bad_alloc &) {
        if (!nothrow)
          throw;
        return NULL;
      }

      mem = do_malloc(size);
    }
    return mem;
  }

  void *do_cpp_alloc(size_t size, bool nothrow) {
    if (NoHook::IsNested())
      return do_cpp_alloc2(size, nothrow);
    NoHook no_hook;
    void *mem = do_cpp_alloc2(size, nothrow);
    Engine::HookAlloc(mem, size);
    return mem;
  }

  void do_cpp_free(void *mem) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested()) {
      do_free(mem);
    } else {
      NoHook no_hook;
      Engine::HookFree(mem);
      do_free(mem);
    }
  }

  void *do_dlopen(const char *file, int mode) {
    return GetDLFunctions().dlopen(file, mode);
  }

  void *do_dlmopen(Lmid_t nsid, const char *file, int mode) {
    return GetDLFunctions().dlmopen(nsid, file, mode);
  }

  int do_dlclose(void *handle) {
    return GetDLFunctions().dlclose(handle);
  }

  int do_dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info,
      size_t size, void *data), void *data) {
    return GetDLFunctions().dl_iterate_phdr(callback, data);
  }

  void *do_dlsym(void *handle, const char *name) {
    return GetDLFunctions().dlsym(handle, name);
  }

  int do_getaddrinfo(const char *node, const char *service,
      const struct addrinfo *hints, struct addrinfo **res) {
    return GetDLFunctions().getaddrinfo(node, service, hints, res);
  }

  int do_getnameinfo(const struct sockaddr *sa, socklen_t salen,
      char *host, socklen_t hostlen, char *serv, socklen_t servlen,
  #if (__GLIBC_MINOR__ <= 12)
      unsigned
  #endif
      int flags) {
    return GetDLFunctions().getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
  }

  struct hostent *do_gethostbyname(const char *name) {
    return GetDLFunctions().gethostbyname(name);
  }

  struct hostent *do_gethostbyaddr(const void *addr, socklen_t len, int type) {
    return GetDLFunctions().gethostbyaddr(addr, len, type);
  }

  struct hostent *do_gethostbyname2(const char *name, int af) {
    return GetDLFunctions().gethostbyname2(name, af);
  }

  int do_gethostent_r(struct hostent *result_buf, char *buf, size_t buflen, struct hostent **result,
      int *h_errnop) {
    return GetDLFunctions().gethostent_r(result_buf, buf, buflen, result, h_errnop);
  }

  int do_gethostbyaddr_r(const void *addr, socklen_t len, int type, struct hostent *result_buf,
      char *buf, size_t buflen, struct hostent **result, int *h_errnop) {
    return GetDLFunctions().gethostbyaddr_r(addr, len, type, result_buf,
        buf, buflen, result, h_errnop);
  }

  int do_gethostbyname_r(const char *name, struct hostent *result_buf, char *buf, size_t buflen,
      struct hostent **result, int *h_errnop) {
    return GetDLFunctions().gethostbyname_r(name, result_buf, buf, buflen,
        result, h_errnop);
  }

  int do_gethostbyname2_r(const char *name, int af, struct hostent *result_buf,
      char *buf, size_t buflen, struct hostent **result, int *h_errnop) {
    return GetDLFunctions().gethostbyname2_r(name, af, result_buf,
        buf, buflen, result, h_errnop);
  }

  int do_getpwent_r(struct passwd *resultbuf, char *buffer, size_t buflen,
      struct passwd **result) {
    return GetDLFunctions().getpwent_r(resultbuf, buffer, buflen, result);
  }

  int do_getpwuid_r(uid_t uid, struct passwd *resultbuf, char *buffer, size_t buflen,
      struct passwd **result) {
    return GetDLFunctions().getpwuid_r(uid, resultbuf, buffer, buflen, result);
  }

  int do_getpwnam_r(const char *name, struct passwd *resultbuf, char *buffer, size_t buflen,
      struct passwd **result) {
    return GetDLFunctions().getpwnam_r(name, resultbuf, buffer, buflen, result);
  }

} // memhook

using namespace memhook;

extern "C" {
  void memhook_free(void *mem) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested()) {
      do_free(mem);
    } else {
      NoHook no_hook;
      Engine::HookFree(mem);
      do_free(mem);
    }
  }

  void *memhook_malloc(size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_malloc(size);
    NoHook no_hook;
    void *mem = do_malloc(size);
    Engine::HookAlloc(mem, size);
    return mem;
  }

  void *memhook_calloc(size_t nmemb, size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_calloc(nmemb, size);
    NoHook no_hook;
    void *mem = do_calloc(nmemb, size);
    Engine::HookAlloc(mem, nmemb * size);
    return mem;
  }

  void *memhook_realloc(void *old_mem, size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_realloc(old_mem, size);
    NoHook no_hook;
    void *new_mem = do_realloc(old_mem, size);
    Engine::HookRealloc(old_mem, new_mem, size);
    return new_mem;
  }

  void *memhook_memalign(size_t alignment, size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_memalign(alignment, size);
    NoHook no_hook;
    void *mem = do_memalign(alignment, size);
    Engine::HookAlloc(mem, size);
    return mem;
  }

  int memhook_posix_memalign(void **memptr, size_t alignment, size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_posix_memalign(memptr, alignment, size);
    NoHook no_hook;
    int rs = do_posix_memalign(memptr, alignment, size);
    Engine::HookAlloc(*memptr, size);
    return rs;
  }

  void memhook_cfree(void *mem) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested()) {
      do_cfree(mem);
    } else {
      NoHook no_hook;
      Engine::HookFree(mem);
      do_cfree(mem);
    }
  }

#if (HAVE_ALIGNED_ALLOC+0)
  void *memhook_aligned_alloc(size_t alignment, size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_aligned_alloc(alignment, size);
    NoHook no_hook;
    void *mem = do_aligned_alloc(alignment, size);
    Engine::HookAlloc(mem, size);
    return mem;
  }
#endif

#if (HAVE_VALLOC+0)
  void *memhook_valloc(size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_valloc(size);
    NoHook no_hook;
    void *mem = do_valloc(size);
    Engine::HookAlloc(mem, size);
    return mem;
  }
#endif

#if (HAVE_PVALLOC+0)
  void *memhook_pvalloc(size_t size) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_pvalloc(size);
    NoHook no_hook;
    void *mem = do_pvalloc(size);
    Engine::HookAlloc(mem, size);
    return mem;
  }
#endif

  void *memhook_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
      MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_mmap(addr, length, prot, flags, fd, offset);
    NoHook no_hook;
    void *mem = do_mmap(addr, length, prot, flags, fd, offset);
    if (fd < 0 && !(flags & MAP_STACK))
      Engine::HookAlloc(mem, length);
    return mem;
  }

  void *memhook_mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset)
      MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_mmap64(addr, length, prot, flags, fd, offset);
    NoHook no_hook;
    void *mem = do_mmap64(addr, length, prot, flags, fd, offset);
    if (fd < 0 && (flags & (MAP_ANONYMOUS | MAP_PRIVATE)) == (MAP_ANONYMOUS | MAP_PRIVATE)) {
      Engine::HookAlloc(mem, length);
    }
    return mem;
  }

  int memhook_munmap(void *addr, size_t length) MEMHOOK_NOEXCEPT {
    if (NoHook::IsNested())
      return do_munmap(addr, length);
    NoHook no_hook;
    int rs = do_munmap(addr, length);
    if (BOOST_LIKELY(rs == 0)) {
      Engine::HookFree(addr);
    }
    return rs;
  }

  void *memhook_mremap(void *addr, size_t old_len, size_t new_len, int flags, ...) MEMHOOK_NOEXCEPT {
    va_list ap;
    va_start(ap, flags);
    void *new_addr_arg = va_arg(ap, void *);
    va_end(ap);
    if (NoHook::IsNested())
      return do_mremap(addr, old_len, new_len, flags, new_addr_arg);
    NoHook no_hook;
    void* new_addr = do_mremap(addr, old_len, new_len, flags, new_addr_arg);
    if (new_addr != MAP_FAILED) {
      Engine::HookRealloc(addr, new_addr, new_len);
    }
    return new_addr;
  }

  void* memhook_new(size_t size) MEMHOOK_THROW(std::bad_alloc) {
    return do_cpp_alloc(size, false);
  }

  void* memhook_new_nothrow(size_t size, const std::nothrow_t&) MEMHOOK_USE_NOEXCEPT {
    return do_cpp_alloc(size, true);
  }

  void memhook_delete(void* p) MEMHOOK_NOEXCEPT {
    do_cpp_free(p);
  }

  void memhook_delete_sized(void* p, size_t) MEMHOOK_NOEXCEPT {
    do_cpp_free(p);
  }

  void memhook_delete_nothrow(void* p, const std::nothrow_t&) MEMHOOK_USE_NOEXCEPT {
    do_cpp_free(p);
  }

  void* memhook_newarray(size_t size) MEMHOOK_THROW(std::bad_alloc) {
    return do_cpp_alloc(size, false);
  }

  void* memhook_newarray_nothrow(size_t size, const std::nothrow_t&) MEMHOOK_USE_NOEXCEPT {
    return do_cpp_alloc(size, true);
  }

  void memhook_deletearray(void* mem) MEMHOOK_USE_NOEXCEPT {
    do_cpp_free(mem);
  }

  void memhook_deletearray_sized(void* mem, size_t) MEMHOOK_THROW(std::bad_alloc) {
    do_cpp_free(mem);
  }

  void memhook_deletearray_nothrow(void* mem, const std::nothrow_t&) MEMHOOK_USE_NOEXCEPT {
    do_cpp_free(mem);
  }

  void *memhook_dlopen(const char *file, int mode) {
    NoHook no_hook;
    return do_dlopen(file, mode);
  }

  void *memhook_dlmopen(Lmid_t nsid, const char *file, int mode) {
    NoHook no_hook;
    return do_dlmopen(nsid, file, mode);
  }

  int memhook_dlclose(void *handle) {
    const int rs = do_dlclose(handle);
    NoHook no_hook;
    Engine::FlushCallStackCache();
    return rs;
  }

  int memhook_dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data),
      void *data) {
    // NoHook no_hook;
    return do_dl_iterate_phdr(callback, data);
  }

  void *memhook_dlsym(void *handle, const char *name) {
    /* dlsym(RTLD_NEXT, "dlsym") -> this function*/
    if (handle == RTLD_NEXT && strcmp(name, "dlsym") == 0)
      return (void *)&memhook_dlsym;

    NoHook no_hook;

    if (BOOST_UNLIKELY(g_dl_functions.dlsym == NULL)) {
      DLFunctions::dlsym_t fn = (DLFunctions::dlsym_t)GLIBC_find_dl_symbol("dlsym");
      if (!fn) {
        dlerror(); /* clear the previous error */

        fn = (DLFunctions::dlsym_t)_dl_sym(RTLD_NEXT, "dlsym",
                MEMHOOK_RETURN_ADDRESS(0)); /* _dl_sym only for RTLD_NEXT */

        const char *const err_s = dlerror();
        if (err_s)
          LogPrintf(kERROR, "_dl_sym(RTLD_NEXT, \"dlsym\") failed: %s\n", err_s);

        if (!fn)
          /* may be unsafe, but does not do abort() */
          return _dl_sym(handle, name, MEMHOOK_RETURN_ADDRESS(0));
      }

      g_dl_functions.dlsym = fn;
    }

    return g_dl_functions.dlsym(handle, name);
  }

#if (HAVE_DLVSYM+0)
  void *memhook_dlvsym(void *handle, const char *name, const char *version) {
    /* dlsym(RTLD_NEXT, "dlsym") -> this function*/
    if (handle == RTLD_NEXT && strcmp(name, "dlvsym") == 0)
      return (void *)&memhook_dlvsym;

    NoHook no_hook;

    if (BOOST_UNLIKELY(g_dl_functions.dlvsym == NULL)) {
      DLFunctions::dlvsym_t fn = (DLFunctions::dlvsym_t)GLIBC_find_dl_symbol("dlvsym");
      if (!fn)
      {
        dlerror(); /* clear the previous error */

        fn = (DLFunctions::dlvsym_t)_dl_sym(RTLD_NEXT, "dlvsym",
                MEMHOOK_RETURN_ADDRESS(0)); /* _dl_sym only for RTLD_NEXT */

        const char *const err_s = dlerror();
        if (err_s)
          LogPrintf(kERROR, "_dl_sym(RTLD_NEXT, \"dlvsym\") failed: %s\n", err_s);

        if (!fn)
          /* may be unsafe, but does not do abort() */
          return _dl_vsym(handle, name, version, MEMHOOK_RETURN_ADDRESS(0));
      }

      g_dl_functions.dlvsym = fn;
    }

    return g_dl_functions.dlvsym(handle, name, version);
  }
#endif

  int memhook_getaddrinfo(const char *node, const char *service,
      const struct addrinfo *hints, struct addrinfo **res) {
    NoHook no_hook;
    return do_getaddrinfo(node, service, hints, res);
  }

  int memhook_getnameinfo(const struct sockaddr *sa, socklen_t salen,
          char *host, socklen_t hostlen, char *serv, socklen_t servlen,
#if (__GLIBC_MINOR__ <= 12)
          unsigned
#endif
          int flags) {
    NoHook no_hook;
    return do_getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
  }

  struct hostent *memhook_gethostbyname(const char *name) {
    NoHook no_hook;
    return do_gethostbyname(name);
  }

  struct hostent *memhook_gethostbyaddr(const void *addr, socklen_t len, int type) {
    NoHook no_hook;
    return do_gethostbyaddr(addr, len, type);
  }

  struct hostent *memhook_gethostbyname2(const char *name, int af) {
    NoHook no_hook;
    return do_gethostbyname2(name, af);
  }

  int memhook_gethostent_r(struct hostent *result_buf, char *buf, size_t buflen,
          struct hostent **result, int *h_errnop) {
    NoHook no_hook;
    return do_gethostent_r(result_buf, buf, buflen, result, h_errnop);
  }

  int memhook_gethostbyaddr_r(const void *addr, socklen_t len, int type, struct hostent *result_buf,
          char *buf, size_t buflen, struct hostent **result, int *h_errnop) {
    NoHook no_hook;
    return do_gethostbyaddr_r(addr, len, type, result_buf, buf, buflen, result, h_errnop);
  }

  int memhook_gethostbyname_r(const char *name, struct hostent *result_buf, char *buf, size_t buflen,
          struct hostent **result, int *h_errnop) {
    NoHook no_hook;
    return do_gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop);
  }

  int memhook_gethostbyname2_r (const char *name, int af, struct hostent *result_buf,
          char *buf, size_t buflen, struct hostent **result, int *h_errnop) {
    NoHook no_hook;
    return do_gethostbyname2_r(name, af, result_buf, buf, buflen, result, h_errnop);
  }

  int memhook_getpwent_r(struct passwd *resultbuf, char *buffer, size_t buflen,
      struct passwd **result) {
    NoHook no_hook;
    return do_getpwent_r(resultbuf, buffer, buflen, result);
  }

  int memhook_getpwuid_r(uid_t uid, struct passwd *resultbuf, char *buffer, size_t buflen,
      struct passwd **result) {
    NoHook no_hook;
    return do_getpwuid_r(uid, resultbuf, buffer, buflen, result);
  }

  int memhook_getpwnam_r(const char *name, struct passwd *resultbuf, char *buffer, size_t buflen,
      struct passwd **result) {
    NoHook no_hook;
    return do_getpwnam_r(name, resultbuf, buffer, buflen, result);
  }

MEMHOOK_API void *malloc(size_t size)                          MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_malloc);
MEMHOOK_API void free(void *ptr)                               MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_free);
MEMHOOK_API void *realloc(void *ptr, size_t size)              MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_realloc);
MEMHOOK_API void *calloc(size_t n, size_t size)                MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_calloc);
MEMHOOK_API void cfree(void *ptr)                              MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_cfree);
MEMHOOK_API void *memalign(size_t align, size_t s)             MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_memalign);
MEMHOOK_API int posix_memalign(void **r, size_t a, size_t s)   MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_posix_memalign);

#if (HAVE_ALIGNED_ALLOC + 0)
MEMHOOK_API void *aligned_alloc(size_t alignment, size_t size) MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_aligned_alloc);
#endif

#if (HAVE_VALLOC + 0)
MEMHOOK_API void *valloc(size_t size)                          MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_valloc);
#endif

#if (HAVE_PVALLOC + 0)
MEMHOOK_API void *pvalloc(size_t size)                         MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_pvalloc);
#endif

MEMHOOK_API void *__libc_malloc(size_t size)                                    MEMHOOK_ALIAS(memhook_malloc);
MEMHOOK_API void __libc_free(void *ptr)                                         MEMHOOK_ALIAS(memhook_free);
MEMHOOK_API void *__libc_realloc(void *ptr, size_t size)                        MEMHOOK_ALIAS(memhook_realloc);
MEMHOOK_API void *__libc_calloc(size_t n, size_t size)                          MEMHOOK_ALIAS(memhook_calloc);
MEMHOOK_API void __libc_cfree(void *ptr)                                        MEMHOOK_ALIAS(memhook_cfree);
MEMHOOK_API void *__libc_memalign(size_t align, size_t s)                       MEMHOOK_ALIAS(memhook_memalign);
MEMHOOK_API int __posix_memalign(void **r, size_t a, size_t s)                  MEMHOOK_ALIAS(memhook_posix_memalign);

#if (HAVE_VALLOC + 0)
MEMHOOK_API void *__libc_valloc(size_t size)                   MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_valloc);
#endif

#if (HAVE_PVALLOC + 0)
MEMHOOK_API void *__libc_pvalloc(size_t size)                  MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_pvalloc);
#endif

MEMHOOK_API void *mmap(void *addr, size_t length, int prot, int flags, int fd,
        off_t offset)                                          MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_mmap);
MEMHOOK_API void *mmap64(void *addr, size_t length, int prot, int flags, int fd,
        off64_t offset)                                        MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_mmap64);
MEMHOOK_API int munmap(void *addr, size_t length)              MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_munmap);
MEMHOOK_API void *mremap(void *addr, size_t old_len, size_t new_len,
        int flags, ...)                                        MEMHOOK_NOEXCEPT MEMHOOK_ALIAS(memhook_mremap);

MEMHOOK_API void *dlopen(const char *file, int mode)                            MEMHOOK_ALIAS(memhook_dlopen);
MEMHOOK_API void *dlmopen(Lmid_t nsid, const char *file, int mode)              MEMHOOK_ALIAS(memhook_dlmopen);
MEMHOOK_API int dlclose(void *handle)                                           MEMHOOK_ALIAS(memhook_dlclose);
MEMHOOK_API int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data),
       void *data)                                                              MEMHOOK_ALIAS(memhook_dl_iterate_phdr);

MEMHOOK_API void *dlsym(void *handle, const char *name)                         MEMHOOK_ALIAS(memhook_dlsym);

#if (HAVE_DLVSYM + 0)
MEMHOOK_API void *dlvsym(void *handle, const char *name, const char *version)   MEMHOOK_ALIAS(memhook_dlvsym);
#endif

MEMHOOK_API int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints,
        struct addrinfo **res)                                                 MEMHOOK_ALIAS(memhook_getaddrinfo);
MEMHOOK_API int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv,
        socklen_t servlen,
#if (__GLIBC_MINOR__ <= 12)
        unsigned
#endif
        int flags)                                                              MEMHOOK_ALIAS(memhook_getnameinfo);
MEMHOOK_API struct hostent *gethostbyaddr(const void *addr,
        socklen_t len, int type)                                                MEMHOOK_ALIAS(memhook_gethostbyaddr);
MEMHOOK_API struct hostent *gethostbyname(const char *name)                     MEMHOOK_ALIAS(memhook_gethostbyname);
MEMHOOK_API struct hostent *gethostbyname2(const char *name, int af)            MEMHOOK_ALIAS(memhook_gethostbyname2);

MEMHOOK_API int gethostent_r(struct hostent *result_buf,
        char *buf,
        size_t buflen,
        struct hostent **result,
        int *h_errnop) MEMHOOK_ALIAS(memhook_gethostent_r);

MEMHOOK_API int gethostbyaddr_r(const void *addr,
        socklen_t len,
        int type,
        struct hostent *result_buf,
        char *buf,
        size_t buflen,
        struct hostent **result,
        int *h_errnop) MEMHOOK_ALIAS(memhook_gethostbyaddr_r);

MEMHOOK_API int gethostbyname_r(const char *name,
        struct hostent *result_buf,
        char *buf,
        size_t buflen,
        struct hostent **result,
        int *h_errnop) MEMHOOK_ALIAS(memhook_gethostbyname_r);

MEMHOOK_API int gethostbyname2_r(const char *name,
        int af,
        struct hostent *result_buf,
        char *buf,
        size_t buflen,
        struct hostent **result,
        int *h_errnop) MEMHOOK_ALIAS(memhook_gethostbyname2_r);

MEMHOOK_API int getpwent_r(struct passwd *resultbuf,
        char *buffer,
        size_t buflen,
        struct passwd **result) MEMHOOK_ALIAS(memhook_getpwent_r);

MEMHOOK_API int getpwuid_r(uid_t uid,
        struct passwd *resultbuf,
        char *buffer,
        size_t buflen,
        struct passwd **result) MEMHOOK_ALIAS(memhook_getpwuid_r);

MEMHOOK_API int getpwnam_r(const char *name,
        struct passwd *resultbuf,
        char *buffer,
        size_t buflen,
        struct passwd **result) MEMHOOK_ALIAS(memhook_getpwnam_r);
} // extern "C"

MEMHOOK_API void* operator new(size_t size)
        MEMHOOK_THROW(std::bad_alloc) MEMHOOK_ALIAS(memhook_new);
MEMHOOK_API void operator delete(void* p)
        MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_delete);
MEMHOOK_API void* operator new[](size_t size)
        MEMHOOK_THROW(std::bad_alloc) MEMHOOK_ALIAS(memhook_newarray);
MEMHOOK_API void operator delete[](void* p)
        MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_deletearray);
MEMHOOK_API void* operator new(size_t size, const std::nothrow_t& nt)
        MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_new_nothrow);
MEMHOOK_API void* operator new[](size_t size, const std::nothrow_t& nt)
        MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_newarray_nothrow);
MEMHOOK_API void operator delete(void* p, const std::nothrow_t& nt)
        MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_delete_nothrow);
MEMHOOK_API void operator delete[](void* p, const std::nothrow_t& nt)
        MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_deletearray_nothrow);

// void operator delete(void *p, size_t size)   MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_delete_sized);
// void operator delete[](void *p, size_t size) MEMHOOK_USE_NOEXCEPT MEMHOOK_ALIAS(memhook_deletearray_sized);

#ifdef BOOST_GCC
#  pragma GCC pop_options
#endif
