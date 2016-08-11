#ifndef MEMHOOK_SRC_MEMHOOK_DLFCN_HOOK_HPP_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_DLFCN_HOOK_HPP_INCLUDED

#include "config.hpp"
#include "critical_section.hpp"
#include "singleton.hpp"
#include "no_hook.hpp"

struct dlfcn_hook;

namespace memhook {

/* we set up own dlfcn_hook for the case when the application uses
 * dlopen(..., RTLD_DEEPBIND | ...) */

class DlfcnHook
    : public SingletonRefCounter<DlfcnHook>
    , public Singleton<DlfcnHook> {
    friend class DlfcnHookSwitch;
public:
    void do_init();
    void do_fini();

    void switch_to_native();
    void switch_to_custom();

private:
    CriticalSection cs_;
    dlfcn_hook *native_;
    dlfcn_hook *custom_;
    uint32_t    hook_depth_;
};

class DlfcnHookSwitch {
    NoHook                          no_hook_;
    boost::intrusive_ptr<DlfcnHook> hook_;
    CriticalSectionLock             lock_;
public:
    DlfcnHookSwitch();
    ~DlfcnHookSwitch();
};

} // ns memhook

#endif
