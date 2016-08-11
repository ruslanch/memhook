#include "dl_functions.hpp"
#include "static_buf_alloc.hpp"
#include "error_msg.hpp"
#include "no_hook.hpp"
#include "glibc.hpp"
#include <boost/scope_exit.hpp>

namespace memhook {

namespace {
    template <typename Sign>
    void dlsym_rtld_next0(Sign *&dlfn, const char *name) {
        dlerror();
        Sign *const fn = (Sign *)_dl_sym(RTLD_NEXT, name, MEMHOOK_RETURN_ADDRESS(0));
        const char *const err_s = dlerror();
        if (err_s != NULL) {
            error_msg("dlsym", err_s);
        }

        if (fn == NULL) {
            abort();
        }

        dlfn = fn;
    }

    template <typename Sign>
    void dlsym_rtld_next(Sign *&dlfn, const char *name) {
        dlerror();
        Sign *const fn = (Sign *)dlsym(RTLD_NEXT, name);
        const char *const err_s = dlerror();
        if (err_s != NULL) {
            error_msg("dlsym", err_s);
        }

        if (fn == NULL) {
            abort();
        }

        dlfn = fn;
    }
} // ns

void DlFunctions::init0() {
    if (this->free) {
        return;
    }

    NoHook no_hook;

    this->free    = &StaticBufAlloc::free;
    this->malloc  = &StaticBufAlloc::malloc;
    this->calloc  = &StaticBufAlloc::calloc;

    dlsym_rtld_next0(this->dlopen,          "dlopen");
    dlsym_rtld_next0(this->dlmopen,         "dlmopen");
    dlsym_rtld_next0(this->dlclose,         "dlclose");
    dlsym_rtld_next0(this->dl_iterate_phdr, "dl_iterate_phdr");

    BOOST_SCOPE_EXIT(this_) {
        this_->dlopen  = NULL;
        this_->dlmopen = NULL;
        this_->dlclose = NULL;
        this_->dl_iterate_phdr = NULL;
    } BOOST_SCOPE_EXIT_END;

    free_t   tmp_free   = NULL;
    malloc_t tmp_malloc = NULL;
    calloc_t tmp_calloc = NULL;
    dlsym_rtld_next(tmp_free,   "free");
    dlsym_rtld_next(tmp_malloc, "malloc");
    dlsym_rtld_next(tmp_calloc, "calloc");
    this->free   = tmp_free;
    this->malloc = tmp_malloc;
    this->calloc = tmp_calloc;

    dlsym_rtld_next(this->realloc,  "realloc");
    dlsym_rtld_next(this->memalign, "memalign");
    dlsym_rtld_next(this->posix_memalign, "posix_memalign");

    dlsym_rtld_next(this->mmap,   "mmap");
    dlsym_rtld_next(this->mmap64, "mmap64");
    dlsym_rtld_next(this->munmap, "munmap");
}

void DlFunctions::init1() {
    init0();

    if (this->dlopen) {
        return;
    }

    NoHook no_hook;

    dlsym_rtld_next(this->dlopen,           "dlopen");
    dlsym_rtld_next(this->dlmopen,          "dlmopen");
    dlsym_rtld_next(this->dlclose,          "dlclose");
    dlsym_rtld_next(this->dl_iterate_phdr,  "dl_iterate_phdr");
}

void DlFunctions::init2() {
    init1();

    if (this->pthread_create) {
        return;
    }

    NoHook no_hook;

    dlsym_rtld_next(this->pthread_create,        "pthread_create");
    dlsym_rtld_next(this->pthread_join,          "pthread_join");
    dlsym_rtld_next(this->pthread_tryjoin_np,    "pthread_tryjoin_np");
    dlsym_rtld_next(this->pthread_timedjoin_np,  "pthread_timedjoin_np");

    dlsym_rtld_next(this->getaddrinfo,      "getaddrinfo");
    dlsym_rtld_next(this->getnameinfo,      "getnameinfo");
    dlsym_rtld_next(this->gethostbyname,    "gethostbyname");
    dlsym_rtld_next(this->gethostbyaddr,    "gethostbyaddr");
    dlsym_rtld_next(this->gethostbyname2,   "gethostbyname2");
    dlsym_rtld_next(this->gethostent_r,     "gethostent_r");
    dlsym_rtld_next(this->gethostbyaddr_r,  "gethostbyaddr_r");
    dlsym_rtld_next(this->gethostbyname_r,  "gethostbyname_r");
    dlsym_rtld_next(this->gethostbyname2_r, "gethostbyname2_r");

    dlsym_rtld_next(this->getpwent_r, "getpwent_r");
    dlsym_rtld_next(this->getpwuid_r, "getpwuid_r");
    dlsym_rtld_next(this->getpwnam_r, "getpwnam_r");
}

} // ns memhook
