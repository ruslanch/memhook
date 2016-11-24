#include "callstack_unwinder.h"
#include "utils.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <dlfcn.h>

#define MEMHOOK_CHECK_UNW(call) \
    if (BOOST_UNLIKELY(call != 0)) { PrintErrorMessage(#call, " failed"); }

namespace memhook
{

static const char unknown_tag[] = "<unknown>";

static inline int GetUnwProcNameByIP(unw_addr_space_t as, unw_word_t ip, char *buf, size_t buf_len, unw_word_t *offp)
{
    unw_accessors_t *a = unw_get_accessors(as);

    if (a->get_proc_name)
        return (*a->get_proc_name)(as, ip, buf, buf_len, offp, 0);

    return -1;
}

CallStackUnwinder::ImplPtr::ImplPtr()
    : ImplTSPtr()
{}

CallStackUnwinder::ImplPtr::~ImplPtr()
{}

class CallStackUnwinder::Impl
{
public:
    Impl()
        : cache_max_size_(8192)
    {
        const char *cache_max_size = getenv("MEMHOOK_CALLSTACK_UNWINDER_CACHE_SIZE");
        if (cache_max_size)
            cache_max_size_ = strtoul(cache_max_size, NULL, 10);
    }

    void GetUnwProcInfo(unw_word_t ip, unw_cursor_t cursor,
            boost::container::string &shl_path,
            unw_word_t &shl_addr,
            boost::container::string &procname,
            unw_word_t &offp)
    {
        UnwProcNameInfoMap::const_iterator iter = procname_info_map_.find(ip);
        if (iter != procname_info_map_.end())
        {
            {
                UnwShlPathMap::const_iterator iter2 = shl_path_map_.find(iter->shl_addr);
                if (iter2 != shl_path_map_.end())
                    shl_path = iter2->second;
                else
                    shl_path = unknown_tag;
            }

            shl_addr = iter->shl_addr;
            procname = iter->procname;
            offp     = iter->offp;

            typename UnwProcNameInfoMap::nth_index<1>::type &index = procname_info_map_.get<1>();
            index.relocate(procname_info_map_.project<1>(iter), index.end());
        }
        else
        {
            boost::array<char, 1024> buf;
            UnwProcNameInfo pni(ip);
            if (unw_get_proc_name(&cursor, buf.data(), buf.size(), &offp) == 0)
            {
                pni.procname = buf.data();
                pni.offp     = offp;
            }
            else
            {
                pni.procname = unknown_tag;
            }

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

            if (procname_info_map_.size() >= cache_max_size_)
            {
                typename UnwProcNameInfoMap::nth_index<1>::type &index = procname_info_map_.get<1>();
                index.erase(index.begin());
            }

            procname_info_map_.insert(procname_info_map_.end(), pni);
        }
    }

   void GetUnwProcInfoByIP(unw_word_t ip,
            boost::container::string &shl_path,
            unw_word_t &shl_addr,
            boost::container::string &procname,
            unw_word_t &offp)
    {
        UnwProcNameInfoMap::const_iterator iter = procname_info_map_.find(ip);
        if (iter != procname_info_map_.end())
        {
            {
                UnwShlPathMap::const_iterator iter2 = shl_path_map_.find(iter->shl_addr);
                if (iter2 != shl_path_map_.end())
                    shl_path = iter2->second;
                else
                    shl_path = unknown_tag;
            }

            shl_addr = iter->shl_addr;
            procname = iter->procname;
            offp     = iter->offp;

            typename UnwProcNameInfoMap::nth_index<1>::type &index = procname_info_map_.get<1>();
            index.relocate(procname_info_map_.project<1>(iter), index.end());
        }
        else
        {
            boost::array<char, 1024> buf;
            UnwProcNameInfo pni(ip);
            if (GetUnwProcNameByIP(unw_local_addr_space, ip, buf.data(), buf.size(), &offp) == 0)
            {
                pni.procname = buf.data();
                pni.offp     = offp;
            }
            else
            {
                pni.procname = unknown_tag;
            }

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

            if (procname_info_map_.size() >= cache_max_size_)
            {
                typename UnwProcNameInfoMap::nth_index<1>::type &index = procname_info_map_.get<1>();
                index.erase(index.begin());
            }

            procname_info_map_.insert(procname_info_map_.end(), pni);
        }
    }

    void FlushUnwCache()
    {
        unw_flush_cache(unw_local_addr_space, 0, 0);

        procname_info_map_.clear();
        shl_path_map_.clear();
    }

private:
    struct UnwProcNameInfo
    {
        boost::container::string procname;
        unw_word_t ip;
        unw_word_t offp;
        unw_word_t shl_addr;
        explicit UnwProcNameInfo(unw_word_t ip = 0)
            : procname(), ip(ip), offp(), shl_addr()
        {}
    };

    typedef boost::multi_index_container<
        UnwProcNameInfo,
        boost::multi_index::indexed_by<
            boost::multi_index::ordered_unique<
                boost::multi_index::member<UnwProcNameInfo, unw_word_t, &UnwProcNameInfo::ip>
            >,
            boost::multi_index::sequenced<>
        >
    > UnwProcNameInfoMap;

    typedef boost::unordered_map<unw_word_t, boost::container::string> UnwShlPathMap;

    std::size_t cache_max_size_;

    UnwProcNameInfoMap procname_info_map_;
    UnwShlPathMap      shl_path_map_;
};

void CallStackUnwinder::Initialize()
{
    unw_caching_policy_t caching_policy = UNW_CACHE_PER_THREAD;

    const char *option = getenv("MEMHOOK_LIBUNWIND_CACHING_POLICY");
    if (option)
    {
        if (option[0] == 'n')
            caching_policy = UNW_CACHE_NONE;
        else if (option[0] == 'g')
            caching_policy = UNW_CACHE_GLOBAL;
    }

    MEMHOOK_CHECK_UNW(unw_set_caching_policy(unw_local_addr_space, caching_policy));
}

void CallStackUnwinder::Destroy()
{
    impl_.reset();
}

void CallStackUnwinder::GetCallStackInfo(CallStackInfo &callstack, size_t skip_frames, bool need_unwind_proc_info)
{
    if (!impl_.get())
    {
        impl_.reset(new Impl());
    }

    callstack.clear();

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

        callstack.push_back(CallStackInfoItem());

        CallStackInfoItem &item = callstack.back();
        item.ip = ip;
        item.sp = sp;

        if (need_unwind_proc_info)
        {
            impl_->GetUnwProcInfo(item.ip, cursor, item.shl_path, item.shl_addr, item.procname, item.offp);
        }
    }
}

void CallStackUnwinder::GetCallStackUnwindProcInfo(CallStackInfo &callstack)
{
    BOOST_FOREACH(CallStackInfoItem &item, callstack)
    {
        impl_->GetUnwProcInfoByIP(item.ip, item.shl_path, item.shl_addr,
                item.procname, item.offp);
    }
}

void CallStackUnwinder::FlushCallStackCache()
{
    impl_->FlushUnwCache();
}

} // memtrace
