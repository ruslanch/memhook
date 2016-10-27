#include "dl_functions.h"
#include "static_buf_alloc.h"
#include "no_hook.h"
#include "utils.h"
#include "glibc.h"

#include <boost/scope_exit.hpp>

namespace memhook
{

namespace
{
    template <typename Sign>
    void DLSymRtldNext0(Sign *&dlfn, const char *name)
    {
        dlerror();
        Sign *const fn = (Sign *)_dl_sym(RTLD_NEXT, name, MEMHOOK_RETURN_ADDRESS(0));
        const char *const err_s = dlerror();
        if (err_s != NULL)
        {
            PrintErrorMessage("dlsym", err_s);
        }

        if (fn == NULL)
        {
            abort();
        }

        dlfn = fn;
    }

    template <typename Sign>
    void DLSymRtldNext(Sign *&dlfn, const char *name)
    {
        dlerror();
        Sign *const fn = (Sign *)dlsym(RTLD_NEXT, name);
        const char *const err_s = dlerror();
        if (err_s != NULL)
        {
            PrintErrorMessage("dlsym", err_s);
        }

        if (fn == NULL)
        {
            abort();
        }

        dlfn = fn;
    }
} // ns

void DLFunctions::Init0()
{
    if (this->free)
    {
        return;
    }

    NoHook no_hook;

    this->free    = &StaticBufAlloc::free;
    this->malloc  = &StaticBufAlloc::malloc;
    this->calloc  = &StaticBufAlloc::calloc;

    DLSymRtldNext0(this->dlopen,          "dlopen");
    DLSymRtldNext0(this->dlmopen,         "dlmopen");
    DLSymRtldNext0(this->dlclose,         "dlclose");
    DLSymRtldNext0(this->dl_iterate_phdr, "dl_iterate_phdr");

    BOOST_SCOPE_EXIT(this_)
    {
        this_->dlopen  = NULL;
        this_->dlmopen = NULL;
        this_->dlclose = NULL;
        this_->dl_iterate_phdr = NULL;
    } BOOST_SCOPE_EXIT_END;

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

    DLSymRtldNext(this->mmap,   "mmap");
    DLSymRtldNext(this->mmap64, "mmap64");
    DLSymRtldNext(this->munmap, "munmap");
}

void DLFunctions::Init1()
{
    Init0();

    if (this->dlopen)
    {
        return;
    }

    NoHook no_hook;

    DLSymRtldNext(this->dlopen,           "dlopen");
    DLSymRtldNext(this->dlmopen,          "dlmopen");
    DLSymRtldNext(this->dlclose,          "dlclose");
    DLSymRtldNext(this->dl_iterate_phdr,  "dl_iterate_phdr");
}

void DLFunctions::Init2()
{
    Init1();

    if (this->pthread_create)
    {
        return;
    }

    NoHook no_hook;

    DLSymRtldNext(this->pthread_create,        "pthread_create");
    DLSymRtldNext(this->pthread_join,          "pthread_join");
    DLSymRtldNext(this->pthread_tryjoin_np,    "pthread_tryjoin_np");
    DLSymRtldNext(this->pthread_timedjoin_np,  "pthread_timedjoin_np");

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
