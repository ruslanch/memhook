#include "dl_fcn_hook.h"
#include "glibc.h"

namespace memhook
{

namespace
{
    struct WrappedFunctionInfo
    {
        const char *name;
        void       *func;
    };

    int dl_iterate_phdr_elfinjection(struct dl_phdr_info *info, size_t size, void *data)
    {
        if (data != NULL)
        {
            if (info->dlpi_name == NULL || strcmp(info->dlpi_name, (const char *)data) != 0)
                return 0;
        }

        if (info->dlpi_name && strstr(info->dlpi_name, MEMHOOK_TARGET_FILE_NAME))
        {
            return 0;
        }

        const intptr_t page_size = getpagesize();

#define MAKE_WRAPPED_FUNCTION_INFO(function) {#function, reinterpret_cast<void *>(&function)}

        static WrappedFunctionInfo s_wrapped_functions[] = {
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
            MAKE_WRAPPED_FUNCTION_INFO(getpwent_r),
            MAKE_WRAPPED_FUNCTION_INFO(getpwuid_r),
            MAKE_WRAPPED_FUNCTION_INFO(getpwnam_r),
            {NULL, NULL}
        };

#undef MAKE_WRAPPED_FUNCTION_INFO

        const ElfW(Phdr) *phdr = info->dlpi_phdr;
        const ElfW(Phdr) *const phdr_end = phdr + info->dlpi_phnum;
        for (; phdr != phdr_end; ++phdr)
        {
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

            for (const ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; ++d)
            {
                switch (d->d_tag)
                {
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

            for (ElfW(Rela) *rel = rel_table; rel < rel_table_end; rel++)
            {
                const ElfW(Word) sym_index =
#ifdef __x86_64__
                    ELF64_R_SYM(rel->r_info)
#else
                    ELF32_R_SYM(rel->r_info)
#endif
                ;

                const char *sym_name = str_table + sym_table[sym_index].st_name;
                void **psym = reinterpret_cast<void**>(rel->r_offset + base);
                for (size_t i = 0; s_wrapped_functions[i].name != NULL; ++i)
                {
                    if (strcmp(s_wrapped_functions[i].name, sym_name) == 0)
                    {
                        if (*psym != s_wrapped_functions[i].func)
                        {
                            void *mem_page = (void *)((intptr_t)psym & ~(page_size - 1));
                            mprotect(mem_page, page_size, PROT_READ | PROT_WRITE);
                            *psym = s_wrapped_functions[i].func;
                        }
                        break;
                    } // if
                } // for
            } // for
        } // for
        return 0;
    } // dl_iterate_phdr_elfinjection

    void *wrap_dlopen(const char *file, int mode, void *dl_caller)
    {
        void *h = NULL;
        {
            DLFcnHookSwitch hook_switch;
            h = dlopen(file, mode/*, dl_caller*/);
        }

        // link_map *map = NULL;
        // dlinfo(h, RTLD_DI_LINKMAP, &map);
        // while (map != NULL)
        // {
        //         fprintf(stderr, "%p: %s\n", map->l_addr, map->l_name);
        //         map = map->l_next;
        // }

        dl_iterate_phdr(dl_iterate_phdr_elfinjection, (void *)file);
        return h;
    }

    void *wrap_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller)
    {
        void *h = NULL;
        {
            DLFcnHookSwitch hook_switch;
            h = dlmopen(nsid, file, mode/*, dl_caller*/);
        }
        dl_iterate_phdr(dl_iterate_phdr_elfinjection, (void *)file);
        return h;
    }

    int wrap_dlclose(void *handle)
    {
        DLFcnHookSwitch hook_switch;
        return dlclose(handle);
    }

    void *wrap_dlsym(void *handle, const char *name, void *dl_caller)
    {
        DLFcnHookSwitch hook_switch;
        return dlsym(handle, name/*, dl_caller*/);
    }

    void *wrap_dlvsym(void *handle, const char *name, const char *version, void *dl_caller)
    {
        DLFcnHookSwitch hook_switch;
        return dlvsym(handle, name, version/*, dl_caller*/);
    }

    char *wrap_dlerror(void)
    {
        DLFcnHookSwitch hook_switch;
        return dlerror();
    }

    int wrap_dladdr(const void *address, Dl_info *info)
    {
        /* do not used DLFcnHookSwitch, used _dl_addr, exported from glibc */
        NoHook no_hook;
        return _dl_addr(address, info, NULL, NULL);
    }

    int wrap_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags)
    {
        /* do not used DLFcnHookSwitch, used _dl_addr, exported from glibc */
        NoHook no_hook;
        switch (flags)
        {
        default:
        case 0:
            return _dl_addr(address, info, NULL, NULL);
        case RTLD_DL_SYMENT:
            return _dl_addr(address, info, NULL, (const void **)extra_info);
        case RTLD_DL_LINKMAP:
            return _dl_addr(address, info, (struct link_map **)extra_info, NULL);
        }
    }

    int wrap_dlinfo(void *handle, int request, void *arg, void *dl_caller)
    {
        DLFcnHookSwitch hook_switch;
        return dlinfo(handle, request, arg/*, dl_caller*/);
    }

    ::dlfcn_hook custom_dlfcn_hook = {
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
} // ns

void DLFcnHook::OnInitialize()
{
    mutex_.Initialize();

    native_ = _dlfcn_hook;
    custom_ = &custom_dlfcn_hook;

    SwitchToCustom();

    hook_depth_ = 0;
}

void DLFcnHook::OnDestroy()
{
    SwitchToNative();
    mutex_.Destroy();
}

void DLFcnHook::SwitchToNative()
{
    _dlfcn_hook = native_;
}

void DLFcnHook::SwitchToCustom()
{
    _dlfcn_hook = custom_;
}

DLFcnHookSwitch::DLFcnHookSwitch()
        : no_hook_()
        , hook_(DLFcnHook::GetInstance())
        , lock_()
{
    if (hook_)
    {
        MutexLock lock(hook_->mutex_);
        if (!hook_->hook_depth_++)
        {
            lock_.swap(lock);
            hook_->SwitchToNative();
        }
    }
}

DLFcnHookSwitch::~DLFcnHookSwitch()
{
    if (!--hook_->hook_depth_)
    {
        hook_->SwitchToCustom();
    }
}

} // ns memhook
