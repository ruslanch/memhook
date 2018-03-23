#ifndef MEMHOOK_SRC_MEMHOOK_ENGINE_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ENGINE_H_INCLUDED

#include "common.h"
#include "callstack_unwinder.h"
#include "singleton.h"
#include "thread.h"

#include <memhook/callstack.h>
#include <memhook/mapped_storage.h>
#include <memhook/traceinfo.h>

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace memhook {
  class Engine : public SingletonImpl<Engine> {
  public:
    struct TraceInfo : TraceInfoBase {
      CallStackInfo callstack;

      TraceInfo(uintptr_t address, size_t memsize, const chrono::system_clock::time_point &timestamp)
          : TraceInfoBase(address, memsize, timestamp)
          , callstack() {}
    };

    void OnInitialize();
    void OnDestroy();

    static void HookAlloc(void *mem, size_t memsize) MEMHOOK_NOEXCEPT;
    static void HookFree(void *mem) MEMHOOK_NOEXCEPT;
    static void HookRealloc(void *old_mem, void *new_mem, size_t new_size) MEMHOOK_NOEXCEPT;
    static void HookUpdateSize(void *mem, size_t newsize) MEMHOOK_NOEXCEPT;
    static void FlushCallStackCache() MEMHOOK_NOEXCEPT;

  protected:
    void DoHookAlloc(void *mem, size_t memsize);
    void DoHookFree(void *mem);
    void DoHookRealloc(void *old_mem, void *new_mem, size_t new_size);
    void DoHookUpdateSize(void *mem, size_t newsize);
    void DoFlushCallStackCache();

  private:
    unique_ptr<MappedStorage> NewStorage() const;

    void FlushLocalCache(const chrono::system_clock::time_point &now,
            const boost::chrono::seconds &timeout,
            bool dump_all);

    unique_ptr<MappedStorage> m_storage;

    typedef boost::multi_index_container<
      TraceInfo,
      boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
          boost::multi_index::member<TraceInfoBase,
            uintptr_t, &TraceInfoBase::address
          >
        >,
        boost::multi_index::ordered_non_unique<
          boost::multi_index::member<TraceInfoBase,
            chrono::system_clock::time_point, &TraceInfoBase::timestamp
          >
        >
      >
    > IndexedContainer;

    Mutex m_cache_mutex;
    IndexedContainer m_cache;

    CallStackUnwinder m_callstack_unwinder;

    chrono::seconds m_cache_flush_timeout;
    std::size_t m_cache_flush_max_items;

    enum CallStackUnwindProcInfoPolicy {
      kUnwindCallStackWhen,
      kFlushLocalCacheWhen,
    };

    CallStackUnwindProcInfoPolicy m_getprocinfo_policy;
  };

}  // ns memhook

#endif
