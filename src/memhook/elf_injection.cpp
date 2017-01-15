#include "elf_injection.h"
#include "glibc.h"

#include <boost/core/ignore_unused.hpp>

namespace memhook {
  namespace {
    struct WrappedFunctionInfo
    {
        const char *name;
        void       *func;
    };

#define MAKE_WRAPPED_FUNCTION_INFO(function) {#function, reinterpret_cast<void *>(&function)}

    WrappedFunctionInfo s_wrapped_functions[] = {
      MAKE_WRAPPED_FUNCTION_INFO(free),
      MAKE_WRAPPED_FUNCTION_INFO(malloc),
      MAKE_WRAPPED_FUNCTION_INFO(calloc),
      MAKE_WRAPPED_FUNCTION_INFO(realloc),
      MAKE_WRAPPED_FUNCTION_INFO(memalign),
      MAKE_WRAPPED_FUNCTION_INFO(posix_memalign),
#if (HAVE_CFREE+0)
      MAKE_WRAPPED_FUNCTION_INFO(cfree),
#endif
#if (HAVE_ALIGNED_ALLOC+0)
      MAKE_WRAPPED_FUNCTION_INFO(aligned_alloc),
#endif
#if (HAVE_VALLOC+0)
      MAKE_WRAPPED_FUNCTION_INFO(valloc),
#endif
#if (HAVE_PVALLOC+0)
      MAKE_WRAPPED_FUNCTION_INFO(pvalloc),
#endif
      MAKE_WRAPPED_FUNCTION_INFO(mmap),
      MAKE_WRAPPED_FUNCTION_INFO(mmap64),
      MAKE_WRAPPED_FUNCTION_INFO(mremap),
      MAKE_WRAPPED_FUNCTION_INFO(munmap),
      MAKE_WRAPPED_FUNCTION_INFO(dlopen),
      MAKE_WRAPPED_FUNCTION_INFO(dlmopen),
      MAKE_WRAPPED_FUNCTION_INFO(dlclose),
      MAKE_WRAPPED_FUNCTION_INFO(dlsym),
      MAKE_WRAPPED_FUNCTION_INFO(dlvsym),
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
      MAKE_WRAPPED_FUNCTION_INFO(getpwent_r),
      MAKE_WRAPPED_FUNCTION_INFO(getpwuid_r),
      MAKE_WRAPPED_FUNCTION_INFO(getpwnam_r),
      {NULL, NULL}
    };
#undef MAKE_WRAPPED_FUNCTION_INFO

    int dl_iterate_phdr_elfinjection(struct dl_phdr_info *info, size_t size, void *data) {
      if (info->dlpi_name && (strstr(info->dlpi_name, MEMHOOK_TARGET_FILE_NAME) ||
                              strstr(info->dlpi_name, "/ld-linux"))) {
        return 0;
      }

      const intptr_t page_size = getpagesize();

      const ElfW(Phdr) *phdr = info->dlpi_phdr;
      const ElfW(Phdr) *const phdr_end = phdr + info->dlpi_phnum;
      for (; phdr != phdr_end; ++phdr) {
        if (phdr->p_type != PT_DYNAMIC)
          continue;

        const ElfW(Dyn) *dyn = reinterpret_cast<const ElfW(Dyn) *>(phdr->p_vaddr + info->dlpi_addr);
        const ElfW(Addr) base = info->dlpi_addr;

        const char *str_table = NULL;
        ElfW(Sym) *sym_table = NULL;
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

        boost::ignore_unused(str_table_size);
        boost::ignore_unused(sym_table_size);

        ElfW(Rela) *const rel_table_end = reinterpret_cast<ElfW(Rela) *>(
                reinterpret_cast<char *>(rel_table) + rel_table_size);

        for (ElfW(Rela) *rel = rel_table; rel < rel_table_end; ++rel) {
          const ElfW(Word) sym_index =
#ifdef __x86_64__
                  ELF64_R_SYM(rel->r_info)
#else
                  ELF32_R_SYM(rel->r_info)
#endif
                  ;

          const char *sym_name = str_table + sym_table[sym_index].st_name;
          void **psym = reinterpret_cast<void **>(rel->r_offset + base);
          for (size_t i = 0; s_wrapped_functions[i].name != NULL; ++i) {
            if (strcmp(s_wrapped_functions[i].name, sym_name) == 0) {
              if (*psym != s_wrapped_functions[i].func) {
                void *mem_page = (void *)((intptr_t)psym & ~(page_size - 1));
                mprotect(mem_page, page_size, PROT_READ | PROT_WRITE);
                *psym = s_wrapped_functions[i].func;
              }
              break;
            }
          }
        }
      }
      return 0;
    }
  }

  void ElfInject() {
    ::dl_iterate_phdr(dl_iterate_phdr_elfinjection, NULL);
  }
}
