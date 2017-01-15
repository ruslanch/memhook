#include "callstack_unwinder.h"
#include "check.h"
#include "thread.h"

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/unordered_map.hpp>

#define UNW_LOCAL_ONLY
#include <dlfcn.h>
#include <libunwind.h>

#define MEMHOOK_UNW_CHECK_SUCCEEDED(_call) MEMHOOK_CHECK((_call) == 0)

namespace memhook {
  static const char unknown_tag[] = "<unknown>";

  static inline int GetUnwProcNameByIP(unw_addr_space_t as, unw_word_t ip,
          char *buf, size_t buf_len, unw_word_t *offp) {
    unw_accessors_t *a = unw_get_accessors(as);

    if (a->get_proc_name)
      return (*a->get_proc_name)(as, ip, buf, buf_len, offp, 0);

    return -1;
  }

  CallStackUnwinder::ImplPtr::ImplPtr()
      : ImplPtrBase() {}

  CallStackUnwinder::ImplPtr::~ImplPtr() {}

  class CallStackUnwinder::Impl {
  public:
    Impl()
        : m_cache_max_size(32768) {
      const char *cache_max_size = getenv("MEMHOOK_CALLSTACK_UNWINDER_CACHE_SIZE");
      if (cache_max_size)
        m_cache_max_size = strtoul(cache_max_size, NULL, 10);
    }

    void GetUnwProcInfo(unw_word_t ip,
            unw_cursor_t cursor,
            boost::container::string &shl_path,
            unw_word_t &shl_addr,
            boost::container::string &procname,
            unw_word_t &offp) {
      MutexLock lock(m_mutex);

      UnwProcNameInfoMap::const_iterator iter = m_procname_info_map.find(ip);
      if (iter != m_procname_info_map.end()) {
        {
          UnwShlPathMap::const_iterator iter2 = m_shl_path_map.find(iter->shl_addr);
          if (iter2 != m_shl_path_map.end())
            shl_path = iter2->second;
          else
            shl_path = unknown_tag;
        }

        shl_addr = iter->shl_addr;
        procname = iter->procname;
        offp = iter->offp;

        UnwProcNameInfoMap::nth_index<1>::type &index = m_procname_info_map.get<1>();
        index.relocate(m_procname_info_map.project<1>(iter), index.end());
      } else {
        lock.Unlock();

        boost::array<char, 1024> buf;
        UnwProcNameInfo pni(ip);
        if (unw_get_proc_name(&cursor, buf.data(), buf.size(), &offp) == 0) {
          pni.procname = buf.data();
          pni.offp = offp;
        } else {
          pni.procname = unknown_tag;
        }

        Dl_info dl_info;
        if (::dladdr(reinterpret_cast<void *>(ip), &dl_info) != 0) {
          pni.shl_addr = (unw_word_t)dl_info.dli_fbase;
          if (dl_info.dli_fname == NULL)
            dl_info.dli_fname = unknown_tag;
          shl_path = dl_info.dli_fname;

          lock.Lock();
          m_shl_path_map.emplace(pni.shl_addr, shl_path);
          lock.Unlock();
        } else {
          shl_path = unknown_tag;
        }

        shl_addr = pni.shl_addr;
        procname = pni.procname;
        offp = pni.offp;

        if (m_procname_info_map.size() >= m_cache_max_size) {
          UnwProcNameInfoMap::nth_index<1>::type &index = m_procname_info_map.get<1>();
          index.erase(index.begin());
        }

        lock.Lock();
        m_procname_info_map.insert(m_procname_info_map.end(), pni);
      }
    }

    void GetUnwProcInfoByIP(unw_word_t ip,
            boost::container::string &shl_path,
            unw_word_t &shl_addr,
            boost::container::string &procname,
            unw_word_t &offp) {
      MutexLock lock(m_mutex);

      UnwProcNameInfoMap::const_iterator iter = m_procname_info_map.find(ip);
      if (iter != m_procname_info_map.end()) {
        {
          UnwShlPathMap::const_iterator iter2 = m_shl_path_map.find(iter->shl_addr);
          if (iter2 != m_shl_path_map.end())
            shl_path = iter2->second;
          else
            shl_path = unknown_tag;
        }

        shl_addr = iter->shl_addr;
        procname = iter->procname;
        offp = iter->offp;

        UnwProcNameInfoMap::nth_index<1>::type &index = m_procname_info_map.get<1>();
        index.relocate(m_procname_info_map.project<1>(iter), index.end());
      } else {
        lock.Unlock();

        boost::array<char, 1024> buf;
        UnwProcNameInfo pni(ip);
        if (GetUnwProcNameByIP(unw_local_addr_space, ip, buf.data(), buf.size(), &offp) == 0) {
          pni.procname = buf.data();
          pni.offp = offp;
        } else {
          pni.procname = unknown_tag;
        }

        Dl_info dl_info;
        if (::dladdr(reinterpret_cast<void *>(ip), &dl_info) != 0) {
          pni.shl_addr = (unw_word_t)dl_info.dli_fbase;
          if (dl_info.dli_fname == NULL)
            dl_info.dli_fname = unknown_tag;
          shl_path = dl_info.dli_fname;

          lock.Lock();
          m_shl_path_map.emplace(pni.shl_addr, shl_path);
          lock.Unlock();
        } else {
          shl_path = unknown_tag;
        }

        shl_addr = pni.shl_addr;
        procname = pni.procname;
        offp = pni.offp;

        lock.Lock();
        if (m_procname_info_map.size() >= m_cache_max_size) {
          UnwProcNameInfoMap::nth_index<1>::type &index = m_procname_info_map.get<1>();
          index.erase(index.begin());
        }

        m_procname_info_map.insert(m_procname_info_map.end(), pni);
      }
    }

    void FlushUnwCache() {
      unw_flush_cache(unw_local_addr_space, 0, 0);

      m_procname_info_map.clear();
      m_shl_path_map.clear();
    }

  private:
    struct UnwProcNameInfo {
      boost::container::string procname;
      unw_word_t ip;
      unw_word_t offp;
      unw_word_t shl_addr;
      explicit UnwProcNameInfo(unw_word_t ip = 0)
          : procname()
          , ip(ip)
          , offp()
          , shl_addr() {}
    };

    typedef boost::multi_index_container<
            UnwProcNameInfo,
            boost::multi_index::indexed_by<
                    boost::multi_index::ordered_unique<boost::multi_index::
                                    member<UnwProcNameInfo, unw_word_t, &UnwProcNameInfo::ip> >,
                    boost::multi_index::sequenced<> > >
            UnwProcNameInfoMap;

    typedef boost::unordered_map<unw_word_t, boost::container::string> UnwShlPathMap;

    std::size_t m_cache_max_size;

    UnwProcNameInfoMap m_procname_info_map;
    UnwShlPathMap m_shl_path_map;

    Mutex m_mutex;
  };

  void CallStackUnwinder::Initialize() {
    unw_caching_policy_t caching_policy = UNW_CACHE_NONE;

    const char *option = getenv("MEMHOOK_LIBUNWIND_CACHING_POLICY");
    if (option) {
      if (option[0] == 't')
        caching_policy = UNW_CACHE_PER_THREAD;
      else if (option[0] == 'g')
        caching_policy = UNW_CACHE_GLOBAL;
    }

    MEMHOOK_UNW_CHECK_SUCCEEDED(unw_set_caching_policy(unw_local_addr_space, caching_policy));

    m_impl.reset(new Impl());
  }

  void CallStackUnwinder::Destroy() {
    m_impl.reset();
  }

  void CallStackUnwinder::GetCallStackInfo(
          CallStackInfo &callstack, size_t skip_frames, bool need_unwind_proc_info) {
    callstack.clear();

    unw_cursor_t cursor;
    unw_context_t uc;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0 && --skip_frames > 0)
      ;

    unw_word_t ip = 0;
    while (unw_step(&cursor) > 0) {
      unw_get_reg(&cursor, UNW_REG_IP, &ip);
      if (BOOST_UNLIKELY(ip == 0))
        break;

      unw_word_t sp = 0;
      unw_get_reg(&cursor, UNW_REG_SP, &sp);

      callstack.push_back(CallStackInfoItem());

      CallStackInfoItem &item = callstack.back();
      item.ip = ip;
      item.sp = sp;

      if (need_unwind_proc_info) {
        m_impl->GetUnwProcInfo(item.ip, cursor, item.shl_path, item.shl_addr,
                item.procname, item.offp);
      }
    }
  }

  void CallStackUnwinder::GetCallStackUnwindProcInfo(CallStackInfo &callstack) {
    BOOST_FOREACH (CallStackInfoItem &item, callstack) {
      m_impl->GetUnwProcInfoByIP(item.ip, item.shl_path, item.shl_addr, item.procname, item.offp);
    }
  }

  void CallStackUnwinder::FlushCallStackCache() {
    m_impl->FlushUnwCache();
  }

}  // memtrace
