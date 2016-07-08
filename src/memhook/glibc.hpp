#ifndef MEMHOOK_SRC_MEMHOOK_GLIBC_HPP_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_GLIBC_HPP_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <dlfcn.h>
#include <link.h> // dl_iterate_phdr
#include <sys/mman.h> // mmap
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifndef MAP_STACK
#   define MAP_STACK 0x20000
#endif

struct dlfcn_hook {
  void *(*dlopen) (const char *file, int mode, void *dl_caller);
  int   (*dlclose)(void *handle);
  void *(*dlsym)  (void *handle, const char *name, void *dl_caller);
  void *(*dlvsym) (void *handle, const char *name, const char *version, void *dl_caller);
  char *(*dlerror)(void);
  int   (*dladdr) (const void *address, Dl_info *info);
  int   (*dladdr1)(const void *address, Dl_info *info, void **extra_info, int flags);
  int   (*dlinfo) (void *handle, int request, void *arg, void *dl_caller);
  void *(*dlmopen)(Lmid_t nsid, const char *file, int mode, void *dl_caller);
  void *pad[4];
};

extern "C" dlfcn_hook *_dlfcn_hook;

// glibc internal function
extern "C" int _dl_addr(const void *address, Dl_info *info, struct link_map **mapp, const void **symbolp);
extern "C" void *_dl_sym(void *handle, const char *name, void *who);
extern "C" void *_dl_vsym(void *handle, const char *name, const char *version, void *who);
extern "C" int _dl_catch_error (const char **objname, const char **errstring, bool *mallocedp,
    void (*operate) (void *), void *args);
extern "C" int _dlerror_run(void (*operate) (void *), void *args);

#endif
