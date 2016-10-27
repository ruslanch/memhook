#ifndef MEMHOOK_SRC_MEMHOOK_DLFCN_HOOK_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_DLFCN_HOOK_H_INCLUDED

#include "common.h"
#include "thread.h"
#include "singleton.h"
#include "no_hook.h"

struct dlfcn_hook;

namespace memhook
{

/* we set up own dlfcn_hook for the case when the application uses
 * dlopen(..., RTLD_DEEPBIND | ...) */

class DLFcnHook : public SingletonImpl<DLFcnHook>
{
    friend class DLFcnHookSwitch;
public:
    void OnInitialize();
    void OnDestroy();

    void SwitchToNative();
    void SwitchToCustom();

private:
    Mutex       mutex_;
    dlfcn_hook *native_;
    dlfcn_hook *custom_;
    uint32_t    hook_depth_;
};

class DLFcnHookSwitch
{
    NoHook                          no_hook_;
    boost::intrusive_ptr<DLFcnHook> hook_;
    MutexLock                       lock_;
public:
    DLFcnHookSwitch();
    ~DLFcnHookSwitch();
};

} // ns memhook

#endif
