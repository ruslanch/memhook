#include "glibc.h"

#include <elf.h>
#include <string.h>

#include "common.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

struct GLIBC_link_map;

extern "C" GLIBC_link_map *_dl_find_dso_for_object(const ElfW(Addr) addr) __attribute__((weak));

struct GLIBC_r_scope_elem {
  struct GLIBC_link_map **r_list;
  unsigned int r_nlist;
};

enum GLIBC_r_dir_status {
  GLIBC_r_dir_status_unknown,
  GLIBC_r_dir_status_nonexisting,
  GLIBC_r_dir_status_existing
};

struct GLIBC_r_search_path_elem {
  struct GLIBC_r_search_path_elem *next;
  const char *what;
  const char *where;
  const char *dirname;
  size_t      dirnamelen;

  enum GLIBC_r_dir_status status[0];
};

struct GLIBC_r_search_path {
  struct GLIBC_r_search_path_elem **dirs;
  int malloced;
};

struct GLIBC_r_found_version {
  const char *name;
  ElfW(Word)  hash;

  int hidden;
  const char *filename;
};

struct GLIBC_r_file_id {
  dev_t   dev;
  ino64_t ino;
};

#if __WORDSIZE == 64
struct GLIBC_link_map_machine {
  Elf64_Addr plt;
  Elf64_Addr gotplt;
  void *tlsdesc_table;
};
#else
struct GLIBC_link_map_machine {
  Elf32_Addr plt;
  Elf32_Addr gotplt;
  void *tlsdesc_table;
};
#endif

struct GLIBC_link_map {
  ElfW(Addr)      l_addr;
  char           *l_name;
  ElfW(Dyn)      *l_ld;
  struct GLIBC_link_map *l_next, *l_prev;
  struct GLIBC_link_map *l_real;
  Lmid_t          l_ns;
  void           *l_libname;

  ElfW(Dyn) *l_info[DT_NUM + 0 + DT_VERSIONTAGNUM + DT_EXTRANUM + DT_VALNUM + DT_ADDRNUM];

  const ElfW(Phdr) *l_phdr;
  ElfW(Addr) l_entry;
  ElfW(Half) l_phnum;
  ElfW(Half) l_ldnum;
  GLIBC_r_scope_elem            l_searchlist;
  GLIBC_r_scope_elem            l_symbolic_searchlist;
  struct GLIBC_link_map        *l_loader;
  struct GLIBC_r_found_version *l_versions;
  unsigned int                  l_nversions;

  Elf_Symndx l_nbuckets;
  Elf32_Word l_gnu_bitmask_idxbits;
  Elf32_Word l_gnu_shift;
  const ElfW(Addr) *l_gnu_bitmask;

  union {
    const Elf32_Word *l_gnu_buckets;
    const Elf_Symndx *l_chain;
  };

  union {
    const Elf32_Word *l_gnu_chain_zero;
    const Elf_Symndx *l_buckets;
  };

  unsigned int l_direct_opencount;

  enum
  {
    lt_executable,
    lt_library,
    lt_loaded
  } l_type : 2;

  unsigned int l_relocated : 1;
  unsigned int l_init_called : 1;
  unsigned int l_global : 1;
  unsigned int l_reserved : 2;
  unsigned int l_phdr_allocated : 1;
  unsigned int l_soname_added : 1;
  unsigned int l_faked : 1;
  unsigned int l_need_tls_init : 1;
  unsigned int l_auditing : 1;
  unsigned int l_audit_any_plt : 1;
  unsigned int l_removed : 1;
  unsigned int l_contiguous : 1;
  unsigned int l_symbolic_in_local_scope : 1;
  unsigned int l_free_initfini : 1;
  struct GLIBC_r_search_path l_rpath_dirs;

  struct reloc_result {
    ElfW(Addr)      addr;
    struct GLIBC_link_map *bound;
    unsigned int    boundndx;
    uint32_t        enterexit;
    unsigned int    flags;
  } *l_reloc_result;

  ElfW(Versym) *l_versyms;

  const char *l_origin;

  ElfW(Addr) l_map_start, l_map_end;
  ElfW(Addr) l_text_end;

  GLIBC_r_scope_elem  *l_scope_mem[4];
  size_t               l_scope_max;
  GLIBC_r_scope_elem **l_scope;
  GLIBC_r_scope_elem  *l_local_scope[2];
  struct GLIBC_r_file_id      l_file_id;
  struct GLIBC_r_search_path  l_runpath_dirs;
  struct GLIBC_link_map     **l_initfini;

  void *l_reldeps;

  unsigned int l_reldepsmax;
  unsigned int l_used;

  ElfW(Word) l_feature_1;
  ElfW(Word) l_flags_1;
  ElfW(Word) l_flags;

  int l_idx;

  struct GLIBC_link_map_machine l_mach;

  struct {
    const ElfW(Sym)        *sym;
    int                     type_class;
    struct GLIBC_link_map  *value;
    const ElfW(Sym)        *ret;
  } l_lookup_cache;

  void     *l_tls_initimage;
  size_t    l_tls_initimage_size;
  size_t    l_tls_blocksize;
  size_t    l_tls_align;
  size_t    l_tls_firstbyte_offset;
  ptrdiff_t l_tls_offset;
  size_t    l_tls_modid;
  size_t    l_tls_dtor_count;

  ElfW(Addr) l_relro_addr;
  size_t     l_relro_size;

  unsigned long long int l_serial;

  struct auditstate {
    uintptr_t    cookie;
    unsigned int bindflags;
  } l_audit[0];
};

struct GLIBC_sym_val {
  const ElfW(Sym)       *s;
  struct GLIBC_link_map *m;
};

static unsigned int GLIBC_dl_elf_hash(const char *name_arg) {
  const unsigned char *name = (const unsigned char *)name_arg;
  unsigned long int hash = 0;
  while (*name != '\0') {
    unsigned long int hi;
    hash = (hash << 4) + *name++;
    hi = hash & 0xf0000000;
    hash ^= hi >> 24;
  }
  hash &= 0x0fffffff;
  return hash;
}

static uint_fast32_t GLIBC_dl_new_hash(const char *s)
{
  uint_fast32_t h = 5381;
  for (unsigned char c = *s; c != '\0'; c = *++s)
    h = h * 33 + c;
  return h & 0xffffffff;
}

static const ElfW(Sym) *GLIBC_check_match(const char *const undef_name,
                                 const ElfW(Sym) *const ref,
                                 const ElfW(Sym) *const sym,
                                 const Elf_Symndx symidx,
                                 const char *const strtab,
                                 const GLIBC_link_map *const map) {
  unsigned int stt = _ElfW(ELF, __ELF_NATIVE_CLASS, ST_TYPE)(sym->st_info);

  if (((1 << stt) & (1 << STT_FUNC)) == 0 || sym->st_value == 0)
    return NULL;

  if (sym != ref && strcmp(strtab + sym->st_name, undef_name))
    return NULL;

  return sym;
}

static int GLIBC_do_lookup(const char *undef_name,
        uint_fast32_t new_hash,
        unsigned long int *old_hash,
        const ElfW(Sym) * ref,
        GLIBC_sym_val *result,
        GLIBC_r_scope_elem *scope,
        size_t i,
        GLIBC_link_map *skip,
        GLIBC_link_map *undef_map) {
  size_t n = scope->r_nlist;
  GLIBC_link_map **list = scope->r_list;

  do {
    const GLIBC_link_map *map = list[i]->l_real;

    if (map == skip)
      continue;

    if (map->l_removed)
      continue;

    if (map->l_nbuckets == 0)
      continue;

    Elf_Symndx symidx;
    int num_versions = 0;

    const ElfW(Sym) *symtab = (ElfW(Sym) *)(map->l_info[DT_SYMTAB]->d_un.d_ptr);
    const char *strtab = (const char *)(map->l_info[DT_STRTAB]->d_un.d_ptr);

    const ElfW(Sym)  *sym = NULL;
    const ElfW(Addr) *bitmask = map->l_gnu_bitmask;
    if (bitmask) {
      ElfW(Addr) bitmask_word = bitmask[(new_hash / __ELF_NATIVE_CLASS) & map->l_gnu_bitmask_idxbits];

      unsigned int hashbit1 = new_hash & (__ELF_NATIVE_CLASS - 1);
      unsigned int hashbit2 = ((new_hash >> map->l_gnu_shift) & (__ELF_NATIVE_CLASS - 1));

      if ((bitmask_word >> hashbit1) & (bitmask_word >> hashbit2) & 1) {
        Elf32_Word bucket = map->l_gnu_buckets[new_hash % map->l_nbuckets];
        if (bucket != 0) {
          const Elf32_Word *hasharr = &map->l_gnu_chain_zero[bucket];

          do {
            if (((*hasharr ^ new_hash) >> 1) == 0) {
              symidx = hasharr - map->l_gnu_chain_zero;
              sym = GLIBC_check_match(undef_name,
                      ref,
                      &symtab[symidx],
                      symidx,
                      strtab,
                      map);
              if (sym != NULL)
                goto sym_found;
            }
          } while ((*hasharr++ & 1u) == 0);
        }
      }

      symidx = SHN_UNDEF;
    } else {
      if (*old_hash == 0xffffffff)
        *old_hash = GLIBC_dl_elf_hash(undef_name);

      for (symidx = map->l_buckets[*old_hash % map->l_nbuckets]; symidx != STN_UNDEF;
              symidx = map->l_chain[symidx]) {
        sym = GLIBC_check_match(undef_name,
                ref,
                &symtab[symidx],
                symidx,
                strtab,
                map);
        if (sym != NULL)
          goto sym_found;
      }
    }

    if (sym != NULL) {
    sym_found:
      switch (_ElfW(ELF, __ELF_NATIVE_CLASS, ST_BIND)(sym->st_info)) {
        case STB_WEAK:
        case STB_GLOBAL:
          result->s = sym;
          result->m = (GLIBC_link_map *)map;
          return 1;
        default:
          break;
      }
    }
  } while (++i < n);

  return 0;
}

static GLIBC_link_map *GLIBC_dl_lookup_symbol(const char *undef_name,
        GLIBC_link_map *undef_map,
        const ElfW(Sym) **ref,
        GLIBC_r_scope_elem *symbol_scope[],
        GLIBC_link_map *skip_map) {
  const uint_fast32_t new_hash = GLIBC_dl_new_hash(undef_name);
  unsigned long int old_hash = 0xffffffff;

  GLIBC_r_scope_elem **scope = symbol_scope;

  size_t i = 0;
  if (skip_map != NULL) {
    while ((*scope)->r_list[i] != skip_map) {
      ++i;
    }
  }

  GLIBC_sym_val current_value = {NULL, NULL};
  for (size_t start = i; *scope != NULL; start = 0, ++scope) {
    int res = GLIBC_do_lookup(undef_name,
            new_hash,
            &old_hash,
            *ref,
            &current_value,
            *scope,
            start,
            skip_map,
            undef_map);
    if (res > 0)
      break;

    if (res < 0 && skip_map == NULL) {
      *ref = NULL;
      return 0;
    }
  }

  if (current_value.s == NULL) {
    *ref = NULL;
    return 0;
  }

  if (current_value.m->l_used == 0)
    current_value.m->l_used = 1;

  *ref = current_value.s;
  return current_value.m;
}

static GLIBC_link_map *GLIBC_dl_find_dso_for_object(const ElfW(Addr) addr)
{
  if (_dl_find_dso_for_object)
    return _dl_find_dso_for_object(addr);
  return NULL;
}

static void *GLIBC_do_sym(const char *name, void *who) {
  ElfW(Addr) caller = (ElfW(Addr))who;
  GLIBC_link_map *l = GLIBC_dl_find_dso_for_object(caller);
  if (!l)
    return NULL;

  GLIBC_link_map *match = l;
  while (l->l_loader != NULL)
    l = l->l_loader;

  const ElfW(Sym) *ref = NULL;
  GLIBC_link_map *map = GLIBC_dl_lookup_symbol(name, match, &ref, l->l_local_scope, match);

  if (!ref)
    return NULL;

  return (void *)(((map) ? (map)->l_addr : 0) + ref->st_value);
}

void *GLIBC_find_dl_symbol(const char *name) {
  return GLIBC_do_sym(name, MEMHOOK_RETURN_ADDRESS(0));
}
