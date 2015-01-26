#include <memhook/common.hpp>
#include <memhook/callstack.hpp>
#include "scoped_use_count.hpp"
#include "mapped_storage.hpp"
#include "error_msg.hpp"
#include <boost/move/unique_ptr.hpp>
#include <boost/array.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <malloc.h>
#include <dlfcn.h>
#include <link.h> // dl_iterate_phdr
#include <sys/mman.h> // mmap

#ifndef MAP_STACK
#   define MAP_STACK 0x20000
#endif

/* glibc internal functions, but exported for me */
extern "C" int _dl_addr(const void *address, Dl_info *info, struct link_map **mapp, const void **symbolp);
extern "C" void *_dl_sym(void *handle, const char *name, void *who);
extern "C" void *_dl_vsym(void *handle, const char *name, const char *version, void *who);
extern "C" int _dl_catch_error (const char **objname, const char **errstring, bool *mallocedp, void (*operate) (void *), void *args);
extern "C" int _dlerror_run(void (*operate) (void *), void *args);

/* glibc internal _dlfcn_hook, but also exported for me */
extern "C" struct dlfcn_hook
{
  void *(*dlopen)  (const char *file, int mode, void *dl_caller);
  int   (*dlclose) (void *handle);
  void *(*dlsym)   (void *handle, const char *name, void *dl_caller);
  void *(*dlvsym)  (void *handle, const char *name, const char *version, void *dl_caller);
  char *(*dlerror) (void);
  int   (*dladdr)  (const void *address, Dl_info *info);
  int   (*dladdr1) (const void *address, Dl_info *info, void **extra_info, int flags);
  int   (*dlinfo)  (void *handle, int request, void *arg, void *dl_caller);
  void *(*dlmopen) (Lmid_t nsid, const char *file, int mode, void *dl_caller);
  void *pad[4];
} *_dlfcn_hook;

namespace memhook
{
    /* defined in callstack.cpp */
    extern void init_callstack();
    extern void fini_callstack();
    extern void get_callstack(callstack_container &callstack);

    /* simple allocator used when the program starts */
    namespace staticbufalloc {
        static char        tmpbuf[1024];
        static std::size_t tmppos = 0;

        static void* malloc(size_t size) BOOST_NOEXCEPT_OR_NOTHROW {
            if (tmppos + size >= sizeof(tmpbuf))
                exit(EXIT_FAILURE);

            void *p = tmpbuf + tmppos;
            tmppos += size;
            return p;
        }

        static void* calloc(size_t nmemb, size_t size) BOOST_NOEXCEPT_OR_NOTHROW {
            const size_t total_size = nmemb * size;
            void *p = malloc(total_size);
            memset(p, 0, total_size);
            return p;
        }

        static void free(void *ptr) BOOST_NOEXCEPT_OR_NOTHROW {
            /* do nothing */
        }
    } // staticbufalloc

#define MEMHOOK_TRY try
#define MEMHOOK_CATCH_ALL \
    catch (const std::exception &e) { error_msg(e.what()); } \
    catch (...) { error_msg("unknown exception"); }

    template <typename Sign>
    void dlsym_rtld_next(Sign **dlfn, const char *name) BOOST_NOEXCEPT_OR_NOTHROW {
        if (BOOST_UNLIKELY(*dlfn == NULL)) {
            dlerror();
            Sign *const fn = (Sign *)dlsym(RTLD_NEXT, name);
            const char *const err_s = dlerror();
            if (BOOST_UNLIKELY(err_s != NULL))
                dlsym_error_msg(err_s);

            if (BOOST_UNLIKELY(fn == NULL))
                abort();

            *dlfn = fn;
        }
    }

    __thread ssize_t hook_depth = 0;

    BOOST_FORCEINLINE
    bool no_hook() { return hook_depth != 0; }

    struct no_hook_this {
        no_hook_this()  BOOST_NOEXCEPT_OR_NOTHROW { ++hook_depth; }
        ~no_hook_this() BOOST_NOEXCEPT_OR_NOTHROW { --hook_depth; }
    };

    static bool is_initstage0 = false;
    static bool is_initstage1 = false;

    typedef void  (*dlfn_free)     (void *);
    typedef void *(*dlfn_malloc)   (size_t);
    typedef void *(*dlfn_calloc)   (size_t, size_t);
    typedef void *(*dlfn_realloc)  (void *, size_t);
    typedef void *(*dlfn_memalign) (size_t, size_t);
    typedef int   (*dlfn_posix_memalign)(void **, size_t, size_t);

    typedef void *(*dlfn_mmap)(void *, size_t, int, int, int, off_t);
    typedef void *(*dlfn_mmap64)(void *, size_t, int, int, int, off64_t);
    typedef int   (*dlfn_munmap)(void *, size_t);

    typedef void *(*dlfn_dlopen)      (const char *, int);
    typedef void *(*dlfn_dlmopen)          (Lmid_t nsid, const char *file, int mode);
    typedef int   (*dlfn_dlclose)     (void *);
    typedef int   (*dlfn_iterate_phdr)(int (*)(struct dl_phdr_info *, size_t, void *), void *);
    typedef void *(*dlfn_dlsym)       (void *, const char *);

    typedef int (*dlfn_getaddrinfo)(const char *, const char *, const struct addrinfo *, struct addrinfo **);
    typedef int (*dlfn_getnameinfo)(const struct sockaddr *, socklen_t, char *, socklen_t, char *, socklen_t, int);
    typedef struct hostent *(*dlfn_gethostbyname)(const char *);
    typedef struct hostent *(*dlfn_gethostbyaddr)(const void *, socklen_t, int);

    typedef struct hostent *(*dlfn_gethostbyname2)(const char *, int);
    typedef int (*dlfn_gethostent_r)(struct hostent *, char *, size_t, struct hostent **, int *);
    typedef int (*dlfn_gethostbyaddr_r)(const void *, socklen_t, int, struct hostent *, char *, size_t, struct hostent **, int *);
    typedef int (*dlfn_gethostbyname_r)(const char *, struct hostent *, char *, size_t, struct hostent **, int *);
    typedef int (*dlfn_gethostbyname2_r)(const char *, int, struct hostent *, char *, size_t, struct hostent **, int *);

    static struct {
        dlfn_free             free;
        dlfn_malloc           malloc;
        dlfn_calloc           calloc;
        dlfn_realloc          realloc;
        dlfn_memalign         memalign;
        dlfn_posix_memalign   posix_memalign;

        dlfn_mmap             mmap;
        dlfn_mmap64           mmap64;
        dlfn_munmap           munmap;

        /* we override these functions because otherwise we get a deadlock on dlopen(...)
         * because to get the callstack libunwind uses dl_iterate_phdr(),
         * which in older versions of glibc uses the same mutex with dlopen */
        dlfn_dlopen           dlopen;
        dlfn_dlmopen          dlmopen;
        dlfn_dlclose          dlclose;
        dlfn_iterate_phdr     dl_iterate_phdr;
        dlfn_dlsym            dlsym;

        /* we override these functions because otherwise we get a deadlock on dlopen(libnss_db.so),
         * in case it is not catched by libmemhook */
        dlfn_getaddrinfo      getaddrinfo;
        dlfn_getnameinfo      getnameinfo;
        dlfn_gethostbyname    gethostbyname;
        dlfn_gethostbyaddr    gethostbyaddr;
        dlfn_gethostbyname2   gethostbyname2;
        dlfn_gethostent_r     gethostent_r;
        dlfn_gethostbyaddr_r  gethostbyaddr_r;
        dlfn_gethostbyname_r  gethostbyname_r;
        dlfn_gethostbyname2_r gethostbyname2_r;
    } dl_function = {0};

    void do_initstage0() BOOST_NOEXCEPT_OR_NOTHROW {
        dl_function.free   = &staticbufalloc::free;
        dl_function.malloc = &staticbufalloc::malloc;
        dl_function.calloc = &staticbufalloc::calloc;

        dlfn_free   tmp_dl_free   = NULL;
        dlfn_malloc tmp_dl_malloc = NULL;
        dlfn_calloc tmp_dl_calloc = NULL;
        dlsym_rtld_next(&tmp_dl_free,   "free");
        dlsym_rtld_next(&tmp_dl_malloc, "malloc");
        dlsym_rtld_next(&tmp_dl_calloc, "calloc");
        dl_function.free   = tmp_dl_free;
        dl_function.malloc = tmp_dl_malloc;
        dl_function.calloc = tmp_dl_calloc;

        dlsym_rtld_next(&dl_function.realloc,  "realloc");
        dlsym_rtld_next(&dl_function.memalign, "memalign");
        dlsym_rtld_next(&dl_function.posix_memalign, "posix_memalign");

        dlsym_rtld_next(&dl_function.mmap,   "mmap");
        dlsym_rtld_next(&dl_function.mmap64, "mmap64");
        dlsym_rtld_next(&dl_function.munmap, "munmap");
    }


    void do_initstage1() BOOST_NOEXCEPT_OR_NOTHROW {
        dlsym_rtld_next(&dl_function.dlopen,          "dlopen");
        dlsym_rtld_next(&dl_function.dlmopen,         "dlmopen");
        dlsym_rtld_next(&dl_function.dlclose,         "dlclose");
        dlsym_rtld_next(&dl_function.dl_iterate_phdr, "dl_iterate_phdr");
        dlsym_rtld_next(&dl_function.getaddrinfo,     "getaddrinfo");
        dlsym_rtld_next(&dl_function.getnameinfo,     "getnameinfo");
        dlsym_rtld_next(&dl_function.gethostbyname,   "gethostbyname");
        dlsym_rtld_next(&dl_function.gethostbyaddr,   "gethostbyaddr");
        dlsym_rtld_next(&dl_function.gethostbyname2,  "gethostbyname2");
        dlsym_rtld_next(&dl_function.gethostent_r,    "gethostent_r");
        dlsym_rtld_next(&dl_function.gethostbyaddr_r, "gethostbyaddr_r");
        dlsym_rtld_next(&dl_function.gethostbyname_r, "gethostbyname_r");
        dlsym_rtld_next(&dl_function.gethostbyname2_r,"gethostbyname2_r");
    }

    BOOST_FORCEINLINE
    void initstage0() BOOST_NOEXCEPT_OR_NOTHROW {
        if (BOOST_UNLIKELY(is_initstage0 == false)) {
            do_initstage0();
            is_initstage0 = true;
        }
    }

    BOOST_FORCEINLINE
    void initstage1() BOOST_NOEXCEPT_OR_NOTHROW {
        initstage0();
        if (BOOST_UNLIKELY(is_initstage1 == false)) {
            do_initstage1();
            is_initstage1 = true;
        }
    }

    static mapped_storage *pctx = NULL;
    static ssize_t         pctx_use_count = 0;

    void init_pctx() try {
        movelib::unique_ptr<mapped_storage> ctx;
        const char *ipc_name = getenv("MEMHOOK_NET_HOST");
        if (ipc_name) {
            int ipc_port = 20015;
            const char *ipc_port_env = getenv("MEMHOOK_NET_PORT");
            if (ipc_port_env)
                ipc_port = strtoul(ipc_port_env, NULL, 10);
            ctx.reset(make_net_storage(ipc_name, ipc_port));
        } else {
            size_t ipc_size = (8ul << 30); // default 8 Gb
            const char *ipc_size_env = getenv("MEMHOOK_SIZE_GB");
            if (ipc_size_env) {
                size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
                if (new_ipc_size != 0)
                    ipc_size = (new_ipc_size << 30);
            } else if ((ipc_size_env = getenv("MEMHOOK_SIZE_MB"))) {
                size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
                if (new_ipc_size != 0)
                    ipc_size = (new_ipc_size << 20);
            } else if ((ipc_size_env = getenv("MEMHOOK_SIZE_KB"))) {
                size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
                if (new_ipc_size != 0)
                    ipc_size = (new_ipc_size << 10);
            } else if ((ipc_size_env = getenv("MEMHOOK_SIZE"))) {
                size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
                if (new_ipc_size != 0)
                    ipc_size = new_ipc_size;
            }

            ipc_name = getenv("MEMHOOK_FILE");
            if (ipc_name) {
                ctx.reset(make_mmf_storage(ipc_name, ipc_size));
            } else {
                ipc_name = getenv("MEMHOOK_SHM_NAME");
                if (!ipc_name)
                    ipc_name = MEMHOOK_SHARED_MEMORY;
                ctx.reset(make_shm_storage(ipc_name, ipc_size));
            }
        }
        pctx = ctx.release();
    } catch (const std::exception &e) {
        error_msg(e.what());
    }

    void fini_pctx() {
        movelib::unique_ptr<mapped_storage> ctx(MEMHOOK_CAS(&pctx, pctx, NULL));
        if (ctx) {
            while (MEMHOOK_CAS(&pctx_use_count, 0, 0) != 0)
                pthread_yield();
        }
    }

    void *wrap_dlopen(const char *file, int mode, void *dl_caller);
    int   wrap_dlclose(void *handle);
    void *wrap_dlsym(void *handle, const char *name, void *dl_caller);
    void *wrap_dlvsym(void *handle, const char *name, const char *version, void *dl_caller);
    char *wrap_dlerror(void);
    int   wrap_dladdr(const void *address, Dl_info *info);
    int   wrap_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags);
    int   wrap_dlinfo(void *handle, int request, void *arg, void *dl_caller);
    void *wrap_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller);
    int dl_iterate_phdr_elfinjection(struct dl_phdr_info *info, size_t size, void *data);

    static dlfcn_hook *default_dlfcn_hook = {0};
    static dlfcn_hook  memhook_dlfcn_hook = {
        wrap_dlopen,
        wrap_dlclose,
        wrap_dlsym,
        wrap_dlvsym,
        wrap_dlerror,
        wrap_dladdr,
        wrap_dladdr1,
        wrap_dlinfo,
        wrap_dlmopen,
        {0, 0, 0, 0},
    };

    static pthread_mutex_t dlfcn_hook_mutex; /* recursive mutex, initializing in init_dlfcn_hook() */
    static ssize_t         dlfcn_depth = 0;

    struct dlfcn_hook_mutex_lock {
        dlfcn_hook_mutex_lock()  { pthread_mutex_lock(&dlfcn_hook_mutex);   }
        ~dlfcn_hook_mutex_lock() { pthread_mutex_unlock(&dlfcn_hook_mutex); }
    };

    struct dlfcn_hook_switch {
        no_hook_this          no_hook_;
        dlfcn_hook_mutex_lock dlfcn_lock_;

        dlfcn_hook_switch() : no_hook_(), dlfcn_lock_() {
            if (!dlfcn_depth++)
                _dlfcn_hook = default_dlfcn_hook;
        }

        ~dlfcn_hook_switch() {
            if (!--dlfcn_depth)
                _dlfcn_hook = &memhook_dlfcn_hook;
        }
    };

#define MEMHOOK_CHECK_PTHREAD(call) \
        if (BOOST_UNLIKELY(call != 0)) { error_msg(#call " failed"); abort(); }

    void init_dlfcn_hook() BOOST_NOEXCEPT_OR_NOTHROW {
        pthread_mutexattr_t dlfcn_hook_mutex_attr;
        MEMHOOK_CHECK_PTHREAD(pthread_mutexattr_init(&dlfcn_hook_mutex_attr));
        MEMHOOK_CHECK_PTHREAD(pthread_mutexattr_settype(&dlfcn_hook_mutex_attr, PTHREAD_MUTEX_RECURSIVE));
        MEMHOOK_CHECK_PTHREAD(pthread_mutex_init(&dlfcn_hook_mutex, &dlfcn_hook_mutex_attr));
        MEMHOOK_CHECK_PTHREAD(pthread_mutexattr_destroy(&dlfcn_hook_mutex_attr));

        dlfcn_hook_mutex_lock lock;
        /* we set up own dlfcn_hook for the case when the application uses
         * dlopen(..., RTLD_DEEPBIND | ...) */
        _dlfcn_hook = &memhook_dlfcn_hook;
    }

    void fini_dlfcn_hook() BOOST_NOEXCEPT_OR_NOTHROW {
        {
            dlfcn_hook_mutex_lock lock;
            _dlfcn_hook = default_dlfcn_hook;
        }
        pthread_mutex_destroy(&dlfcn_hook_mutex);
    }

    static bool initall_done = false;
    void initall() BOOST_NOEXCEPT_OR_NOTHROW {
        if (BOOST_LIKELY(initall_done))
            return;

        initall_done = true;
        initstage0();
        initstage1();
    }

    void finiall() BOOST_NOEXCEPT_OR_NOTHROW {
        if (BOOST_LIKELY(!initall_done))
            return;
    }

    MEMHOOK_SYMBOL_INIT(101)
    void memhook_init() BOOST_NOEXCEPT_OR_NOTHROW {
        no_hook_this no_hook_this;
        initall();

        init_callstack();
        init_pctx();
        init_dlfcn_hook();
    }

    MEMHOOK_SYMBOL_FINI(101)
    void memhook_fini() BOOST_NOEXCEPT_OR_NOTHROW {
        no_hook_this no_hook_this;
        fini_dlfcn_hook();
        fini_pctx();
        fini_callstack();

        finiall();
    }

    void wrap_free(void *mem) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        if (BOOST_LIKELY(mem != NULL)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                ctx->erase(reinterpret_cast<uintptr_t>(mem));
            }
        }
        dl_function.free(mem);
    } MEMHOOK_CATCH_ALL

    void *wrap_malloc(size_t size) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        void *const mem = dl_function.malloc(size);
        if (BOOST_LIKELY(mem != NULL)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(mem), size, callstack);
            }
        }
        return mem;
    } MEMHOOK_CATCH_ALL

    void *wrap_calloc(size_t nmemb, size_t size) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        void *const mem = dl_function.calloc(nmemb, size);
        if (BOOST_LIKELY(mem != NULL)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(mem), size, callstack);
            }
        }
        return mem;
    } MEMHOOK_CATCH_ALL

    void *wrap_memalign(size_t alignment, size_t size) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        void *const mem = dl_function.memalign(alignment, size);
        if (BOOST_LIKELY(mem != NULL)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(mem), size, callstack);
            }
        }
        return mem;
    } MEMHOOK_CATCH_ALL

    int wrap_posix_memalign(void **memptr, size_t alignment, size_t size) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        const int ret = dl_function.posix_memalign(memptr, alignment, size);
        if (BOOST_LIKELY(ret == 0 && *memptr != NULL)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(*memptr), size, callstack);
            }
        }
        return ret;
    } MEMHOOK_CATCH_ALL

    void *wrap_realloc(void *mem, size_t size) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        void *const memnew = dl_function.realloc(mem, size);
        const scoped_use_count use_count(&pctx_use_count);
        mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
        if (BOOST_LIKELY(ctx != NULL)) {
            if (mem) {
                if (!memnew || memnew != mem)
                    ctx->erase(reinterpret_cast<uintptr_t>(mem));
                else if (mem == memnew)
                    ctx->update_size(reinterpret_cast<uintptr_t>(mem), size);
            }

            if ((!mem && memnew) || (memnew && mem != memnew)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(memnew), size, callstack);
            }
        }
        return memnew;
    } MEMHOOK_CATCH_ALL

    void *wrap_mmap(void *addr, size_t size, int prot, int flags,
            int fd, off_t offset) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        void *const mem = dl_function.mmap(addr, size, prot, flags, fd, offset);
        // const int allowed_flags = MAP_ANONYMOUS | MAP_PRIVATE;
        if (BOOST_LIKELY(mem && fd < 0 && (flags & MAP_PRIVATE)
                /*addr == NULL && (flags & (allowed_flags | MAP_STACK)) == allowed_flags*/)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(mem), size, callstack);
            }
        }
        return mem;
    } MEMHOOK_CATCH_ALL

    void *wrap_mmap64(void *addr, size_t size, int prot, int flags,
            int fd, off64_t offset) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        void *const mem = dl_function.mmap64(addr, size, prot, flags, fd, offset);
        // const int allowed_flags = MAP_ANONYMOUS | MAP_PRIVATE;
        if (BOOST_LIKELY(mem && fd < 0 && (flags & MAP_PRIVATE)
                /*addr == NULL && (flags & (allowed_flags | MAP_STACK)) == allowed_flags*/)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                callstack_container callstack;
                get_callstack(callstack);
                ctx->insert(reinterpret_cast<uintptr_t>(mem), size, callstack);
            }
        }
        return mem;
    } MEMHOOK_CATCH_ALL

    int wrap_munmap(void *mem, size_t size) BOOST_NOEXCEPT_OR_NOTHROW MEMHOOK_TRY {
        if (BOOST_LIKELY(mem != NULL)) {
            const scoped_use_count use_count(&pctx_use_count);
            mapped_storage *const ctx = MEMHOOK_CAS(&pctx, NULL, NULL);
            if (BOOST_LIKELY(ctx != NULL)) {
                ctx->erase(reinterpret_cast<uintptr_t>(mem));
            }
        }
        return dl_function.munmap(mem, size);
    } MEMHOOK_CATCH_ALL

    void *wrap_dlopen(const char *file, int mode, void *dl_caller) {
        void *h = NULL;
        {
            dlfcn_hook_switch hook_switch;
            h = dlopen(file, mode/*, dl_caller*/);
        }

        // link_map *map = NULL;
        // dlinfo(h, RTLD_DI_LINKMAP, &map);
        // while (map != NULL) {
        //         fprintf(stderr, "%p: %s\n", map->l_addr, map->l_name);
        //         map = map->l_next;
        // }

        dl_iterate_phdr(dl_iterate_phdr_elfinjection, (void *)file);
        return h;
    }

    void *wrap_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller) {
        void *h = NULL;
        {
            dlfcn_hook_switch hook_switch;
            h = dlmopen(nsid, file, mode/*, dl_caller*/);
        }
        dl_iterate_phdr(dl_iterate_phdr_elfinjection, (void *)file);
        return h;
    }

    int wrap_dlclose(void *handle) {
        dlfcn_hook_switch hook_switch;
        return dlclose(handle);
    }

    void *wrap_dlsym(void *handle, const char *name, void *dl_caller) {
        dlfcn_hook_switch hook_switch;
        return dlsym(handle, name/*, dl_caller*/);
    }

    void *wrap_dlvsym(void *handle, const char *name, const char *version, void *dl_caller) {
        dlfcn_hook_switch hook_switch;
        return dlvsym(handle, name, version/*, dl_caller*/);
    }

    char *wrap_dlerror(void) {
        dlfcn_hook_switch hook_switch;
        return dlerror();
    }

    int wrap_dladdr(const void *address, Dl_info *info) {
        /* do not used dlfcn_hook_switch, used _dl_addr, exported from glibc */
        no_hook_this no_hook_this;
        return _dl_addr(address, info, NULL, NULL);
    }

    int wrap_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags) {
        /* do not used dlfcn_hook_switch, used _dl_addr, exported from glibc */
        no_hook_this no_hook_this;
        switch (flags) {
        default:
        case 0:
            return _dl_addr(address, info, NULL, NULL);
        case RTLD_DL_SYMENT:
            return _dl_addr(address, info, NULL, (const void **)extra_info);
        case RTLD_DL_LINKMAP:
            return _dl_addr(address, info, (struct link_map **)extra_info, NULL);
        }
    }

    int wrap_dlinfo(void *handle, int request, void *arg, void *dl_caller) {
        dlfcn_hook_switch hook_switch;
        return dlinfo(handle, request, arg/*, dl_caller*/);
    }
} // memhook

#pragma GCC push_options
#pragma GCC optimize("no-optimize-sibling-calls")

using namespace memhook;

extern "C" MEMHOOK_API
void free(void *mem) BOOST_NOEXCEPT_OR_NOTHROW {
    if (no_hook() && dl_function.free)
        return dl_function.free(mem);

    no_hook_this no_hook_this;
    initall();
    return wrap_free(mem);
}

extern "C" MEMHOOK_API
void *malloc(size_t size) BOOST_NOEXCEPT_OR_NOTHROW {
    if (no_hook() && dl_function.malloc)
        return dl_function.malloc(size);

    no_hook_this no_hook_this;
    initall();
    return wrap_malloc(size);
}


extern "C" MEMHOOK_API
void *calloc(size_t nmemb, size_t size) BOOST_NOEXCEPT_OR_NOTHROW {
    if (no_hook() && dl_function.calloc)
        return dl_function.calloc(nmemb, size);

    no_hook_this no_hook_this;
    initall();
    return wrap_calloc(nmemb, size);
}

extern "C" MEMHOOK_API
void *realloc(void *mem, size_t newsize) BOOST_NOEXCEPT_OR_NOTHROW {
    if (no_hook() && dl_function.realloc)
        return dl_function.realloc(mem, newsize);

    no_hook_this no_hook_this;
    initall();
    return wrap_realloc(mem, newsize);
}

extern "C" MEMHOOK_API
void *memalign(size_t alignment, size_t size) BOOST_NOEXCEPT_OR_NOTHROW {
    if (no_hook() && dl_function.memalign)
        return dl_function.memalign(alignment, size);

    no_hook_this no_hook_this;
    initall();
    return wrap_memalign(alignment, size);
}

extern "C" MEMHOOK_API
int posix_memalign(void **memptr, size_t alignment, size_t size) BOOST_NOEXCEPT_OR_NOTHROW {
    if (no_hook() && dl_function.posix_memalign)
        return dl_function.posix_memalign(memptr, alignment, size);

    no_hook_this no_hook_this;
    initall();
    return wrap_posix_memalign(memptr, alignment, size);
}

extern "C" MEMHOOK_API
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (no_hook() && dl_function.mmap)
        return dl_function.mmap(addr, length, prot, flags, fd, offset);

    no_hook_this no_hook_this;
    initall();
    return wrap_mmap(addr, length, prot, flags, fd, offset);
}

extern "C" MEMHOOK_API
void *mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset) {
    if (no_hook() && dl_function.mmap)
        return dl_function.mmap64(addr, length, prot, flags, fd, offset);

    no_hook_this no_hook_this;
    initall();
    return wrap_mmap64(addr, length, prot, flags, fd, offset);
}

extern "C" MEMHOOK_API
int munmap(void *addr, size_t length) {
    if (no_hook() && dl_function.munmap)
        return dl_function.munmap(addr, length);

    no_hook_this no_hook_this;
    initall();
    return wrap_munmap(addr, length);
}

extern "C" MEMHOOK_API
void *dlopen(const char *file, int mode) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.dlopen(file, mode);
}

extern "C" MEMHOOK_API
void *dlmopen(Lmid_t nsid, const char *file, int mode) {
    no_hook_this no_hook_this;
    initall();
    return dl_function.dlmopen(nsid, file, mode);
}

extern "C" MEMHOOK_API
int dlclose(void *handle) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.dlclose(handle);
}

extern "C" MEMHOOK_API
int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data),
        void *data) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.dl_iterate_phdr(callback, data);
}

extern "C" MEMHOOK_API
void *dlsym(void *handle, const char *name) BOOST_NOEXCEPT_OR_NOTHROW {
    /* dlsym(RTLD_NEXT, "dlsym") -> this function*/
    if (handle == RTLD_NEXT && strcmp(name, "dlsym") == 0)
        return (void *)&dlsym;

    no_hook_this no_hook_this;
    if (BOOST_UNLIKELY(dl_function.dlsym == NULL)) {
        dlerror(); /* clear the previous error */
        const dlfn_dlsym dlsym_fn = (dlfn_dlsym)_dl_sym(RTLD_NEXT, "dlsym",
            MEMHOOK_RETURN_ADDRESS(0)); /* _dl_sym only for RTLD_NEXT */
        const char *const err_s = dlerror();
        if (BOOST_UNLIKELY(err_s != NULL))
            dlsym_error_msg(err_s);

        if (BOOST_UNLIKELY(dlsym_fn == NULL))
            return _dl_sym(handle, name,
                MEMHOOK_RETURN_ADDRESS(0)); /* may be unsafe, but does not abort() */

        dl_function.dlsym = dlsym_fn;
    }
    return dl_function.dlsym(handle, name);
}

extern "C" MEMHOOK_API
int getaddrinfo(const char *node, const char *service,
        const struct addrinfo *hints, struct addrinfo **res) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.getaddrinfo(node, service, hints, res);
}

extern "C" MEMHOOK_API
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
        char *host, socklen_t hostlen, char *serv, socklen_t servlen,
        int flags) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
}

extern "C" MEMHOOK_API
struct hostent *gethostbyname(const char *name) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostbyname(name);
}

extern "C" MEMHOOK_API
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostbyaddr(addr, len, type);
}

extern "C" MEMHOOK_API
struct hostent *gethostbyname2 (const char *name, int af) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostbyname2(name, af);
}

extern "C" MEMHOOK_API
int gethostent_r(struct hostent *result_buf, char *buf, size_t buflen, struct hostent **result,
        int *h_errnop) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostent_r(result_buf, buf, buflen, result, h_errnop);
}

extern "C" MEMHOOK_API
int gethostbyaddr_r(const void *addr, socklen_t len, int type, struct hostent *result_buf,
        char *buf, size_t buflen, struct hostent **result, int *h_errnop) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostbyaddr_r(addr, len, type, result_buf, buf, buflen, result, h_errnop);
}

extern "C" MEMHOOK_API
int gethostbyname_r(const char *name, struct hostent *result_buf, char *buf, size_t buflen,
        struct hostent **result, int *h_errnop) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop);
}

extern "C" MEMHOOK_API
int gethostbyname2_r (const char *name, int af, struct hostent *result_buf,
        char *buf, size_t buflen, struct hostent **result, int *h_errnop) BOOST_NOEXCEPT_OR_NOTHROW {
    no_hook_this no_hook_this;
    initall();
    return dl_function.gethostbyname2_r(name, af, result_buf, buf, buflen, result, h_errnop);
}

namespace memhook {
    struct wrapped_function_info {
        const char *name;
        void       *func;
    };

    int dl_iterate_phdr_elfinjection(struct dl_phdr_info *info, size_t size, void *data) {
        if (data != NULL) {
            if (info->dlpi_name == NULL || strcmp(info->dlpi_name, (const char *)data) != 0)
                return 0;
        }

        if (info->dlpi_name && strstr(info->dlpi_name, MEMHOOK_TARGET_FILE_NAME)) {
            return 0;
        }

        const intptr_t page_size = getpagesize();

#define MAKE_WRAPPED_FUNCTION_INFO(function) {#function, reinterpret_cast<void *>(&function)}
        static array<wrapped_function_info, 28> dl_wrapped_functions = {{
            MAKE_WRAPPED_FUNCTION_INFO(free),
            MAKE_WRAPPED_FUNCTION_INFO(malloc),
            MAKE_WRAPPED_FUNCTION_INFO(calloc),
            MAKE_WRAPPED_FUNCTION_INFO(realloc),
            MAKE_WRAPPED_FUNCTION_INFO(memalign),
            MAKE_WRAPPED_FUNCTION_INFO(posix_memalign),
            MAKE_WRAPPED_FUNCTION_INFO(mmap),
            MAKE_WRAPPED_FUNCTION_INFO(mmap64),
            MAKE_WRAPPED_FUNCTION_INFO(munmap),
            MAKE_WRAPPED_FUNCTION_INFO(dlopen),
            MAKE_WRAPPED_FUNCTION_INFO(dlclose),
            MAKE_WRAPPED_FUNCTION_INFO(dlsym),
            MAKE_WRAPPED_FUNCTION_INFO(dlvsym),
            MAKE_WRAPPED_FUNCTION_INFO(dlerror),
            MAKE_WRAPPED_FUNCTION_INFO(dladdr),
            MAKE_WRAPPED_FUNCTION_INFO(dladdr1),
            MAKE_WRAPPED_FUNCTION_INFO(dlinfo),
            MAKE_WRAPPED_FUNCTION_INFO(dlmopen),
            MAKE_WRAPPED_FUNCTION_INFO(dl_iterate_phdr),
            MAKE_WRAPPED_FUNCTION_INFO(getaddrinfo),
            MAKE_WRAPPED_FUNCTION_INFO(getnameinfo),
            MAKE_WRAPPED_FUNCTION_INFO(gethostbyname),
            MAKE_WRAPPED_FUNCTION_INFO(gethostbyaddr),
            MAKE_WRAPPED_FUNCTION_INFO(gethostbyname2),
            MAKE_WRAPPED_FUNCTION_INFO(gethostent_r),
            MAKE_WRAPPED_FUNCTION_INFO(gethostbyaddr_r),
            MAKE_WRAPPED_FUNCTION_INFO(gethostbyname_r),
            MAKE_WRAPPED_FUNCTION_INFO(gethostbyname2_r),
        }};
#undef MAKE_WRAPPED_FUNCTION_INFO

        const ElfW(Phdr) *phdr = info->dlpi_phdr;
        const ElfW(Phdr) *const phdr_end = phdr + info->dlpi_phnum;
        for (; phdr != phdr_end; ++phdr) {
            if (phdr->p_type != PT_DYNAMIC)
                continue;

            const ElfW(Dyn) *dyn  = reinterpret_cast<const ElfW(Dyn) *>(phdr->p_vaddr + info->dlpi_addr);
            const ElfW(Addr) base = info->dlpi_addr;

            const char *str_table = NULL;
            ElfW(Sym)  *sym_table = NULL;
            ElfW(Rela) *rel_table = NULL;
            ElfW(Xword) str_table_size = 0;
            ElfW(Xword) sym_table_size = 0;
            ElfW(Xword) rel_table_size = 0;

            for (const ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; ++d) {
                switch (d->d_tag) {
                case DT_STRTAB:
                    str_table = reinterpret_cast<const char *>(d->d_un.d_ptr);
                    break;
                case DT_JMPREL:
                    rel_table = reinterpret_cast<ElfW(Rela) *>(d->d_un.d_ptr);
                    break;
                case DT_SYMTAB:
                    sym_table = reinterpret_cast<ElfW(Sym) *>(d->d_un.d_ptr);
                    break;
                case DT_STRSZ:
                    str_table_size = d->d_un.d_val;
                    break;
                case DT_PLTRELSZ:
                    rel_table_size = d->d_un.d_val;
                    break;
                case DT_SYMENT:
                    sym_table_size = d->d_un.d_val;
                    break;
                }
            }

            ElfW(Rela) *const rel_table_end = reinterpret_cast<ElfW(Rela) *>(
                reinterpret_cast<char *>(rel_table) + rel_table_size);
            for (ElfW(Rela) *rel = rel_table; rel < rel_table_end; rel++) {
                const ElfW(Word) sym_index =
#ifdef __x86_64__
                    ELF64_R_SYM(rel->r_info)
#else
                    ELF32_R_SYM(rel->r_info)
#endif
                ;

                const char *sym_name = str_table + sym_table[sym_index].st_name;
                void **psym = reinterpret_cast<void**>(rel->r_offset + base);
                for (size_t i = 0; i < dl_wrapped_functions.size(); ++i) {
                    if (strcmp(dl_wrapped_functions[i].name, sym_name) == 0) {
                        if (*psym != dl_wrapped_functions[i].func) {
                            void *mem_page = (void *)((intptr_t)psym & ~(page_size - 1));
                            mprotect(mem_page, page_size, PROT_READ | PROT_WRITE);
                            *psym = dl_wrapped_functions[i].func;
                        }
                        break;
                    } // if
                } // for
            } // for
        } // for
        return 0;
   } // dl_iterate_phdr_elfinjection
} // memhook

#pragma GCC pop_options
