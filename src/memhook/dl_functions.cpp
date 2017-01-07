#include "dl_functions.h"
#include "static_buf_alloc.h"
#include "no_hook.h"
#include "check.h"
#include "glibc.h"

#include <boost/scope_exit.hpp>

namespace memhook {
  namespace {
    template <typename F>
    void DLSymRtldNext0(F *&dlfn, const char *name) {
      dlerror();
      F *const fn = (F *)_dl_sym(RTLD_NEXT, name, MEMHOOK_RETURN_ADDRESS(0));
      const char *const err_s = dlerror();
      if (BOOST_UNLIKELY(err_s != NULL))
        LogPrintf(kERROR, "dlsym() failed: %s\n", err_s);
      MEMHOOK_EXPECT(fn != NULL);
      dlfn = fn;
    }

    template <typename F>
    void DLSymRtldNext(F *&dlfn, const char *name) {
      dlerror();
      F *const fn = (F *)dlsym(RTLD_NEXT, name);
      const char *const err_s = dlerror();
      if (BOOST_UNLIKELY(err_s != NULL))
        LogPrintf(kERROR, "dlsym() failed: %s\n", err_s);
      MEMHOOK_EXPECT(fn != NULL);
      dlfn = fn;
    }
  } // ns

  void DLFunctions::DoInitialize() {
    if (BOOST_LIKELY(this->free != NULL))
      return;

    this->free    = &StaticBufAlloc::free;
    this->malloc  = &StaticBufAlloc::malloc;
    this->calloc  = &StaticBufAlloc::calloc;

    DLSymRtldNext0(this->dlopen,          "dlopen");
    DLSymRtldNext0(this->dlmopen,         "dlmopen");
    DLSymRtldNext0(this->dlclose,         "dlclose");
    DLSymRtldNext0(this->dl_iterate_phdr, "dl_iterate_phdr");

    free_t   tmp_free   = NULL;
    malloc_t tmp_malloc = NULL;
    calloc_t tmp_calloc = NULL;
    DLSymRtldNext(tmp_free,   "free");
    DLSymRtldNext(tmp_malloc, "malloc");
    DLSymRtldNext(tmp_calloc, "calloc");
    this->free   = tmp_free;
    this->malloc = tmp_malloc;
    this->calloc = tmp_calloc;

    DLSymRtldNext(this->realloc,  "realloc");
    DLSymRtldNext(this->memalign, "memalign");
    DLSymRtldNext(this->posix_memalign, "posix_memalign");

    #if (HAVE_CFREE+0)
    DLSymRtldNext(this->cfree, "cfree");
    #endif

    #if (HAVE_ALIGNED_ALLOC+0)
    DLSymRtldNext(this->aligned_alloc, "aligned_alloc");
    #endif

    #if (HAVE_VALLOC+0)
    DLSymRtldNext(this->valloc, "valloc");
    #endif

    #if (HAVE_PVALLOC+0)
    DLSymRtldNext(this->pvalloc, "pvalloc");
    #endif

    DLSymRtldNext(this->mmap,   "mmap");
    DLSymRtldNext(this->mmap64, "mmap64");
    DLSymRtldNext(this->munmap, "munmap");

    DLSymRtldNext(this->dlopen,           "dlopen");
    DLSymRtldNext(this->dlmopen,          "dlmopen");
    DLSymRtldNext(this->dlclose,          "dlclose");
    DLSymRtldNext(this->dl_iterate_phdr,  "dl_iterate_phdr");

    DLSymRtldNext(this->getaddrinfo,      "getaddrinfo");
    DLSymRtldNext(this->getnameinfo,      "getnameinfo");
    DLSymRtldNext(this->gethostbyname,    "gethostbyname");
    DLSymRtldNext(this->gethostbyaddr,    "gethostbyaddr");
    DLSymRtldNext(this->gethostbyname2,   "gethostbyname2");
    DLSymRtldNext(this->gethostent_r,     "gethostent_r");
    DLSymRtldNext(this->gethostbyaddr_r,  "gethostbyaddr_r");
    DLSymRtldNext(this->gethostbyname_r,  "gethostbyname_r");
    DLSymRtldNext(this->gethostbyname2_r, "gethostbyname2_r");

    DLSymRtldNext(this->getpwent_r, "getpwent_r");
    DLSymRtldNext(this->getpwuid_r, "getpwuid_r");
    DLSymRtldNext(this->getpwnam_r, "getpwnam_r");
  }
} // ns memhook
