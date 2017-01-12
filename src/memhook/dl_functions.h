#ifndef MEMHOOK_SRC_MEMHOOK_DL_FUNCTIONS_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_DL_FUNCTIONS_H_INCLUDED

#include "common.h"

#include <dlfcn.h>
#include <link.h>  // dl_iterate_phdr
#include <netdb.h>
#include <pwd.h>
#include <sys/mman.h>  // mmap
#include <sys/types.h>

namespace memhook {
  class DLFunctions {
  public:
    enum LinkerInitialized { kLINKER_INITIALIZED };

    DLFunctions(LinkerInitialized x) {}

    typedef void (*free_t)(void *);
    typedef void *(*malloc_t)(size_t);
    typedef void *(*calloc_t)(size_t, size_t);
    typedef void *(*realloc_t)(void *, size_t);
    typedef void *(*memalign_t)(size_t, size_t);
    typedef int (*posix_memalign_t)(void **, size_t, size_t);

    typedef void *(*mmap_t)(void *, size_t, int, int, int, off_t);
    typedef void *(*mmap64_t)(void *, size_t, int, int, int, off64_t);
    typedef void *(*mremap_t)(void *, size_t, size_t, int, ...);
    typedef int (*munmap_t)(void *, size_t);

    typedef void *(*dlopen_t)(const char *, int);
    typedef void *(*dlmopen_t)(Lmid_t nsid, const char *file, int mode);
    typedef int (*dlclose_t)(void *);
    typedef int (*iterate_phdr_t)(int (*)(struct dl_phdr_info *, size_t, void *), void *);
    typedef void *(*dlsym_t)(void *, const char *);
    typedef void *(*dlvsym_t)(void *, const char *, const char *);

    typedef int (*pthread_create_t)(pthread_t *thread,
            const pthread_attr_t *attr,
            void *(*start_routine)(void *),
            void *arg);
    typedef int (*pthread_join_t)(pthread_t thread, void **retval);
    typedef int (*pthread_tryjoin_np_t)(pthread_t thread, void **retval);
    typedef int (*pthread_timedjoin_np_t)(
            pthread_t thread, void **retval, const struct timespec *abstime);
    typedef int (*getaddrinfo_t)(
            const char *, const char *, const struct addrinfo *, struct addrinfo **);
    typedef int (*getnameinfo_t)(const struct sockaddr *,
            socklen_t,
            char *,
            socklen_t,
            char *,
            socklen_t,
#if (__GLIBC_MINOR__ <= 12)
            unsigned
#endif
            int);
    typedef struct hostent *(*gethostbyname_t)(const char *);
    typedef struct hostent *(*gethostbyaddr_t)(const void *, socklen_t, int);
    typedef struct hostent *(*gethostbyname2_t)(const char *, int);
    typedef int (*gethostent_r_t)(struct hostent *, char *, size_t, struct hostent **, int *);
    typedef int (*gethostbyaddr_r_t)(const void *,
            socklen_t,
            int,
            struct hostent *,
            char *,
            size_t,
            struct hostent **,
            int *);
    typedef int (*gethostbyname_r_t)(
            const char *, struct hostent *, char *, size_t, struct hostent **, int *);
    typedef int (*gethostbyname2_r_t)(
            const char *, int, struct hostent *, char *, size_t, struct hostent **, int *);
    typedef int (*getpwent_r_t)(struct passwd *, char *, size_t, struct passwd **);
    typedef int (*getpwuid_r_t)(uid_t, struct passwd *, char *, size_t, struct passwd **);
    typedef int (*getpwnam_r_t)(const char *, struct passwd *, char *, size_t, struct passwd **);

    free_t free;
    malloc_t malloc;
    calloc_t calloc;
    realloc_t realloc;
    memalign_t memalign;
    posix_memalign_t posix_memalign;

#if (HAVE_CFREE + 0)
    typedef void (*cfree_t)(void *);
    cfree_t cfree;
#endif

#if (HAVE_ALIGNED_ALLOC + 0)
    typedef void *(*aligned_alloc_t)(size_t, size_t);
    aligned_alloc_t aligned_alloc;
#endif

#if (HAVE_VALLOC + 0)
    typedef void *(*valloc_t)(size_t);
    valloc_t valloc;
#endif

#if (HAVE_PVALLOC + 0)
    typedef void *(*pvalloc_t)(size_t);
    pvalloc_t pvalloc;
#endif

    mmap_t mmap;
    mmap64_t mmap64;
    mremap_t mremap;
    munmap_t munmap;

    /* we override these functions because otherwise we get a deadlock on dlopen(...)
     * because to get the callstack libunwind uses dl_iterate_phdr(),
     * which in older versions of glibc uses the same mutex with dlopen */
    dlopen_t dlopen;
    dlmopen_t dlmopen;
    dlclose_t dlclose;
    iterate_phdr_t dl_iterate_phdr;
    dlsym_t dlsym;
#if (HAVE_DLVSYM + 0)
    dlvsym_t dlvsym;
#endif

    /* we override these functions because otherwise we get a deadlock on dlopen(libnss_db.so),
     * in case it is not catched by libmemhook */
    getaddrinfo_t getaddrinfo;
    getnameinfo_t getnameinfo;
    gethostbyname_t gethostbyname;
    gethostbyaddr_t gethostbyaddr;
    gethostbyname2_t gethostbyname2;
    gethostent_r_t gethostent_r;
    gethostbyaddr_r_t gethostbyaddr_r;
    gethostbyname_r_t gethostbyname_r;
    gethostbyname2_r_t gethostbyname2_r;

    getpwent_r_t getpwent_r;
    getpwuid_r_t getpwuid_r;
    getpwnam_r_t getpwnam_r;

    void Initialize() {
      if (BOOST_LIKELY(this->free == NULL))
        DoInitialize();
    }

  private:
    void DoInitialize();
  };
}  // ns memhook

#endif
