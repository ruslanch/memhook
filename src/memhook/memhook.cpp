#include "common.h"
#include "no_hook.h"
#include "utils.h"
#include "glibc.h"
#include "dl_functions.h"
#include "dl_fcn_hook.h"
#include "engine.h"

namespace memhook
{
    DLFunctions dl_functions = {0};

    MEMHOOK_SYMBOL_INIT(101)
    void MemHookInit()
    {
        NoHook no_hook;

        dl_functions.Init1();

        Engine::Initialize();
        DLFcnHook::Initialize();
    }

    MEMHOOK_SYMBOL_FINI(101)
    void MemHookFini()
    {
        NoHook no_hook;

        DLFcnHook::Destroy();
        Engine::Destroy();
    }

    int GetThreadStackInfo(pthread_t thread, void **addr, size_t *size)
    {
        pthread_attr_t attr;
        int ret = pthread_getattr_np(thread, &attr);
        if (ret == 0)
            ret = pthread_attr_getstack(&attr, addr, size);
        return ret;
    }

    void CatchAlloc(void *mem, std::size_t size)
    {
        boost::intrusive_ptr<Engine> engine(Engine::GetInstance());
        if (engine)
            engine->Insert(mem, size);
    }

    void CatchFree(void *mem)
    {
        boost::intrusive_ptr<Engine> engine(Engine::GetInstance());
        if (engine)
            engine->Erase(mem);
    }

    void CatchUpdateSize(void *mem, size_t size)
    {
        boost::intrusive_ptr<Engine> engine(Engine::GetInstance());
        if (engine)
            engine->UpdateSize(mem, size);
    }

    void FlushCallStackCache()
    {
        boost::intrusive_ptr<Engine> engine(Engine::GetInstance());
        if (engine)
            engine->FlushCallStackCache();
    }

} // memhook

#pragma GCC push_options
#pragma GCC optimize("no-optimize-sibling-calls")

using namespace memhook;

#define MEMHOOK_FUNCTION_INIT(initfn) \
    dl_functions.initfn()

#define MEMHOOK_FUNCTION_CALL(call) \
    dl_functions.call

#define MEMHOOK_FUNCTION_PROLOGUE(initfn, call) \
    MEMHOOK_FUNCTION_INIT(initfn); \
    if (NoHook::IsNested()) return MEMHOOK_FUNCTION_CALL(call); \
    NoHook no_hook__;

#define MEMHOOK_FUNCTION_CALL_AND_RETURN(initfn, call) \
    MEMHOOK_FUNCTION_INIT(initfn); \
    NoHook no_hook__; \
    return MEMHOOK_FUNCTION_CALL(call);

extern "C"
void free(void *mem)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, free(mem));

    if (BOOST_LIKELY(mem != NULL))
        CatchFree(mem);
    MEMHOOK_FUNCTION_CALL(free(mem));
}

extern "C"
void *malloc(size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, malloc(size));

    void *const mem = MEMHOOK_FUNCTION_CALL(malloc(size));
    if (BOOST_LIKELY(mem != NULL))
        CatchAlloc(mem, size);
    return mem;
}

extern "C"
void *calloc(size_t nmemb, size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, calloc(nmemb, size));

    void *const mem = MEMHOOK_FUNCTION_CALL(calloc(nmemb, size));
    if (BOOST_LIKELY(mem != NULL))
        CatchAlloc(mem, nmemb * size);
    return mem;
}

extern "C"
void *realloc(void *mem, size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, realloc(mem, size));

    void *const memnew = MEMHOOK_FUNCTION_CALL(realloc(mem, size));

    if (mem)
    {
        if (!memnew || memnew != mem)
        {
            CatchFree(mem);
        }
        else if (memnew == mem)
        {
            CatchUpdateSize(mem, size);
        }
    }

    if ((memnew && !mem) || (memnew && memnew != mem))
    {
        CatchAlloc(memnew, size);
    }

    return memnew;
}

extern "C"
void *memalign(size_t alignment, size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, memalign(alignment, size));

    void *const mem = MEMHOOK_FUNCTION_CALL(memalign(alignment, size));
    if (BOOST_LIKELY(mem != NULL))
        CatchAlloc(mem, size);
    return mem;
}

extern "C"
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, posix_memalign(memptr, alignment, size));

    const int ret = MEMHOOK_FUNCTION_CALL(posix_memalign(memptr, alignment, size));
    if (BOOST_LIKELY(ret == 0 && *memptr != NULL))
        CatchAlloc(*memptr, size);
    return ret;
}

#if (HAVE_CFREE+0)
extern "C"
void cfree(void *mem)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, cfree(mem));

    if (BOOST_LIKELY(mem != NULL))
        CatchFree(mem);
    MEMHOOK_FUNCTION_CALL(cfree(mem));
}
#endif

#if (HAVE_ALIGNED_ALLOC+0)
extern "C"
void *aligned_alloc(size_t alignment, size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, aligned_alloc(alignment, size));

    void *const mem = MEMHOOK_FUNCTION_CALL(aligned_alloc(alignment, size));
    if (BOOST_LIKELY(mem != NULL))
        CatchAlloc(mem, size);
    return mem;
}
#endif

#if (HAVE_VALLOC+0)
extern "C"
void *valloc(size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, valloc(size));

    void *const mem = MEMHOOK_FUNCTION_CALL(valloc(size));
    if (BOOST_LIKELY(mem != NULL))
        CatchAlloc(mem, size);
    return mem;
}
#endif

#if (HAVE_PVALLOC+0)
extern "C"
void *pvalloc(size_t size)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, pvalloc(size));

    void *const mem = MEMHOOK_FUNCTION_CALL(pvalloc(size));
    if (BOOST_LIKELY(mem != NULL))
        CatchAlloc(mem, size);
    return mem;
}
#endif

extern "C"
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, mmap(addr, length, prot, flags, fd, offset));

    void *const mem = MEMHOOK_FUNCTION_CALL(mmap(addr, length, prot, flags, fd, offset));
    const int allowed_flags = MAP_ANONYMOUS | MAP_PRIVATE;
    if (BOOST_LIKELY(mem && fd < 0 && (flags & (allowed_flags | MAP_STACK)) == allowed_flags))
        CatchAlloc(mem, length);
    return mem;
}

extern "C"
void *mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, mmap64(addr, length, prot, flags, fd, offset));

    void *const mem = MEMHOOK_FUNCTION_CALL(mmap64(addr, length, prot, flags, fd, offset));
    const int allowed_flags = MAP_ANONYMOUS | MAP_PRIVATE;
    if (BOOST_LIKELY(mem && fd < 0 && (flags & (allowed_flags | MAP_STACK)) == allowed_flags))
        CatchAlloc(mem, length);
    return mem;
}

extern "C"
int munmap(void *addr, size_t length)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init0, munmap(addr, length));

    if (BOOST_LIKELY(addr != NULL))
        CatchFree(addr);

    return MEMHOOK_FUNCTION_CALL(munmap(addr, length));
}

extern "C"
void *dlopen(const char *file, int mode)
{
    MEMHOOK_FUNCTION_INIT(Init1);
    return MEMHOOK_FUNCTION_CALL(dlopen(file, mode));
}

extern "C"
void *dlmopen(Lmid_t nsid, const char *file, int mode)
{
    MEMHOOK_FUNCTION_INIT(Init1);
    return MEMHOOK_FUNCTION_CALL(dlmopen(nsid, file, mode));
}

extern "C"
int dlclose(void *handle)
{
    MEMHOOK_FUNCTION_INIT(Init1);

    int ret = MEMHOOK_FUNCTION_CALL(dlclose(handle));

    NoHook no_hook;
    FlushCallStackCache();

    return ret;
}

extern "C"
int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data),
        void *data)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init1, dl_iterate_phdr(callback, data));
}

extern "C"
void *dlsym(void *handle, const char *name)
{
    /* dlsym(RTLD_NEXT, "dlsym") -> this function*/
    if (handle == RTLD_NEXT && strcmp(name, "dlsym") == 0)
    {
        return (void *)&dlsym;
    }

    NoHook no_hook;
    if (BOOST_UNLIKELY(dl_functions.dlsym == NULL))
    {
        dlerror(); /* clear the previous error */
        const DLFunctions::dlsym_t fn = (DLFunctions::dlsym_t)_dl_sym(RTLD_NEXT, "dlsym",
            MEMHOOK_RETURN_ADDRESS(0)); /* _dl_sym only for RTLD_NEXT */

        const char *const err_s = dlerror();
        if (err_s)
        {
            PrintErrorMessage("dlsym", err_s);
        }

        if (!fn)
        {
            /* may be unsafe, but does not abort() */
            return _dl_sym(handle, name, MEMHOOK_RETURN_ADDRESS(0));
        }

        dl_functions.dlsym = fn;
    }

    return MEMHOOK_FUNCTION_CALL(dlsym(handle, name));
}

extern "C"
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *),
        void *arg)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init2, pthread_create(thread, attr, start_routine, arg));

    const int ret = MEMHOOK_FUNCTION_CALL(pthread_create(thread, attr, start_routine, arg));
    if (ret == 0)
    {
        void  *stack_addr = NULL;
        size_t stack_size = 0;
        if (GetThreadStackInfo(*thread, &stack_addr, &stack_size) == 0)
            CatchAlloc(stack_addr, stack_size);
    }
    return ret;
}

extern "C"
int pthread_join(pthread_t thread, void **retval)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init2, pthread_join(thread, retval));

    void  *stack_addr = NULL;
    size_t stack_size = 0;
    const int stack_ret = GetThreadStackInfo(thread, &stack_addr, &stack_size);
    const int tjoin_ret = MEMHOOK_FUNCTION_CALL(pthread_join(thread, retval));
    if (stack_ret == 0 && tjoin_ret == 0)
        CatchFree(stack_addr);
    return tjoin_ret;
}

extern "C"
int pthread_tryjoin_np(pthread_t thread, void **retval)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init2, pthread_tryjoin_np(thread, retval));

    void  *stack_addr = NULL;
    size_t stack_size = 0;
    const int stack_ret = GetThreadStackInfo(thread, &stack_addr, &stack_size);
    const int tjoin_ret = MEMHOOK_FUNCTION_CALL(pthread_tryjoin_np(thread, retval));
    if (stack_ret == 0 && tjoin_ret == 0)
        CatchFree(stack_addr);
    return tjoin_ret;
}

extern "C"
int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime)
{
    MEMHOOK_FUNCTION_PROLOGUE(Init2, pthread_timedjoin_np(thread, retval, abstime));

    void  *stack_addr = NULL;
    size_t stack_size = 0;
    const int stack_ret = GetThreadStackInfo(thread, &stack_addr, &stack_size);
    const int tjoin_ret = MEMHOOK_FUNCTION_CALL(pthread_timedjoin_np(thread, retval, abstime));
    if (stack_ret == 0 && tjoin_ret == 0)
        CatchFree(stack_addr);
    return tjoin_ret;
}

extern "C"
int getaddrinfo(const char *node, const char *service,
        const struct addrinfo *hints, struct addrinfo **res)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, getaddrinfo(node, service, hints, res));
}

extern "C"
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
        char *host, socklen_t hostlen, char *serv, socklen_t servlen,
#if (__GLIBC_MINOR__ <= 12)
        unsigned
#endif
        int flags)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2,
        getnameinfo(sa, salen, host, hostlen, serv, servlen, flags));
}

extern "C"
struct hostent *gethostbyname(const char *name)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, gethostbyname(name));
}

extern "C"
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, gethostbyaddr(addr, len, type));
}

extern "C"
struct hostent *gethostbyname2(const char *name, int af)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, gethostbyname2(name, af));
}

extern "C"
int gethostent_r(struct hostent *result_buf, char *buf, size_t buflen, struct hostent **result,
        int *h_errnop)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, gethostent_r(result_buf, buf, buflen, result, h_errnop));
}

extern "C"
int gethostbyaddr_r(const void *addr, socklen_t len, int type, struct hostent *result_buf,
        char *buf, size_t buflen, struct hostent **result, int *h_errnop)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2,
        gethostbyaddr_r(addr, len, type, result_buf, buf, buflen, result, h_errnop));
}

extern "C"
int gethostbyname_r(const char *name, struct hostent *result_buf, char *buf, size_t buflen,
        struct hostent **result, int *h_errnop)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2,
        gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop));
}

extern "C"
int gethostbyname2_r (const char *name, int af, struct hostent *result_buf,
        char *buf, size_t buflen, struct hostent **result, int *h_errnop)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2,
        gethostbyname2_r(name, af, result_buf, buf, buflen, result, h_errnop));
}

extern "C"
int getpwent_r(struct passwd *resultbuf, char *buffer, size_t buflen, struct passwd **result)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, getpwent_r(resultbuf, buffer, buflen, result));
}

extern "C"
int getpwuid_r(uid_t uid, struct passwd *resultbuf, char *buffer, size_t buflen,
        struct passwd **result)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, getpwuid_r(uid, resultbuf, buffer, buflen, result));
}

extern "C"
int getpwnam_r(const char *name, struct passwd *resultbuf, char *buffer, size_t buflen,
        struct passwd **result)
{
    MEMHOOK_FUNCTION_CALL_AND_RETURN(Init2, getpwnam_r(name, resultbuf, buffer, buflen, result));
}

#pragma GCC pop_options
