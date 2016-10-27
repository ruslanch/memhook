#include "callstack_unwinder.h"
#include "utils.h"

#include <boost/unordered_map.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/array.hpp>
#include <boost/scope_exit.hpp>

#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <dlfcn.h>

#define MEMHOOK_CHECK_UNW(call) \
    if (BOOST_UNLIKELY(call != 0)) { PrintErrorMessage(#call, " failed"); }

namespace memhook
{

static const char unknown_tag[] = "<unknown>";

class CallStackUnwinder::Impl
{
public:
    Impl()
    {
        MEMHOOK_CHECK_UNW(unw_set_caching_policy(unw_local_addr_space, UNW_CACHE_PER_THREAD));
    }

    void GetUnwProcInfo(unw_word_t ip, unw_cursor_t cursor,
            boost::container::string &shl_path,
            unw_word_t &shl_addr,
            boost::container::string &procname,
            unw_word_t &offp)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        UnwProcNameInfoMap::const_iterator iter = procname_info_map_.find(ip);
        if (iter != procname_info_map_.end())
        {
            UnwShlPathMap::const_iterator iter2 = shl_path_map_.find(iter->second.shl_addr);
            if (iter2 != shl_path_map_.end())
                shl_path = iter2->second;
            else
                shl_path = unknown_tag;
            lock.unlock();
            shl_addr = iter->second.shl_addr;
            procname = iter->second.procname;
            offp     = iter->second.offp;
        }
        else
        {
            lock.unlock();
            boost::array<char, 1024> buf;
            UnwProcNameInfo pni;
            if (unw_get_proc_name(&cursor, buf.data(), buf.size(), &offp) == 0)
            {
                pni.procname = buf.data();
                pni.offp     = offp;
            }
            else
            {
                pni.procname = unknown_tag;
            }
            lock.lock();

            Dl_info dl_info;
            if (::dladdr(reinterpret_cast<void *>(ip), &dl_info) != 0)
            {
                pni.shl_addr = (unw_word_t)dl_info.dli_fbase;
                if (dl_info.dli_fname == NULL)
                    dl_info.dli_fname = unknown_tag;
                shl_path = dl_info.dli_fname;
                shl_path_map_.emplace(pni.shl_addr, shl_path);
            }
            else
            {
                shl_path = unknown_tag;
            }

            shl_addr = pni.shl_addr;
            procname = pni.procname;
            offp     = pni.offp;

            procname_info_map_.emplace(ip, pni);
        }
    }

    void FlushUnwCache()
    {
        unw_flush_cache(unw_local_addr_space, 0, 0);
    }

private:
    struct UnwProcNameInfo
    {
        boost::container::string procname;
        unw_word_t offp;
        unw_word_t shl_addr;
        UnwProcNameInfo() : procname(), offp(), shl_addr()
        {}
    };

    typedef boost::unordered_map<unw_word_t, UnwProcNameInfo>          UnwProcNameInfoMap;
    typedef boost::unordered_map<unw_word_t, boost::container::string> UnwShlPathMap;

    boost::mutex       mutex_;
    UnwProcNameInfoMap procname_info_map_;
    UnwShlPathMap      shl_path_map_;
};

CallStackUnwinder::ImplPtr::ImplPtr()
    : ImplUniquePtr()
{}

CallStackUnwinder::ImplPtr::ImplPtr(BOOST_RV_REF(ImplUniquePtr) p)
    : ImplUniquePtr(boost::move(p))
{}

CallStackUnwinder::ImplPtr::~ImplPtr()
{}

void CallStackUnwinder::Initialize()
{
    ImplPtr impl(make_unique<Impl>());
    impl_.swap(impl);
}

void CallStackUnwinder::Destroy()
{
    impl_.reset();
}

void CallStackUnwinder::GetCallStackInfo(CallStackInfo &callstack, size_t skip_frames)
{
    callstack.clear();
    callstack.reserve(16);

    unw_cursor_t cursor; unw_context_t uc;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0 && --skip_frames > 0)
        ;

    boost::container::string shl_path, procname;
    unw_word_t ip = 0;
    while (unw_step(&cursor) > 0)
    {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        if (BOOST_UNLIKELY(ip == 0))
            break;

        unw_word_t sp = 0;
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        unw_word_t offp = 0, shl_addr = 0;
        impl_->GetUnwProcInfo(ip, cursor, shl_path, shl_addr, procname, offp);
        callstack.push_back(CallStackInfoItem());

        CallStackInfoItem &item = callstack.back();
        item.shl_path.swap(shl_path);
        item.procname.swap(procname);
        item.shl_addr = shl_addr;
        item.ip       = ip;
        item.sp       = sp;
        item.offp     = offp;
    }
}

void CallStackUnwinder::FlushCallStackCache()
{
    impl_->FlushUnwCache();
}

} // memtrace
