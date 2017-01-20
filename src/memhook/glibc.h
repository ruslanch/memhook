#ifndef MEMHOOK_SRC_MEMHOOK_GLIBC_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_GLIBC_H_INCLUDED

#include <dlfcn.h>
#include <link.h> // dl_iterate_phdr
#include <malloc.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/mman.h> // mmap
#include <sys/socket.h>
#include <sys/types.h>

#ifndef MAP_STACK
#define MAP_STACK 0x20000
#endif

extern "C" {

struct dlfcn_hook {
  void *(*dlopen)(const char *file, int mode, void *dl_caller);
  int (*dlclose)(void *handle);
  void *(*dlsym)(void *handle, const char *name, void *dl_caller);
  void *(*dlvsym)(void *handle, const char *name, const char *version, void *dl_caller);
  char *(*dlerror)(void);
  int (*dladdr)(const void *address, Dl_info *info);
  int (*dladdr1)(const void *address, Dl_info *info, void **extra_info, int flags);
  int (*dlinfo)(void *handle, int request, void *arg, void *dl_caller);
  void *(*dlmopen)(Lmid_t nsid, const char *file, int mode, void *dl_caller);
  void *pad[4];
};

extern dlfcn_hook *_dlfcn_hook;

// glibc internal function
int _dl_addr(const void *address, Dl_info *info, struct link_map **mapp, const void **symbolp);
void *_dl_sym(void *handle, const char *name, void *who);
void *_dl_vsym(void *handle, const char *name, const char *version, void *who);

}  // extern "C"

void *GLIBC_find_dl_symbol(const char *name);

#endif
