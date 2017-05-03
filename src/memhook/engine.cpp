#include "engine.h"
#include "log.h"
#include "no_hook.h"

#include <memhook/chrono_utils.h>

namespace memhook {
  void Engine::OnInitialize() {
    try {
      unique_ptr<MappedStorage> storage(NewStorage());
      if (storage)
        m_storage.swap(storage);
    } catch (const std::exception &e) {
      LogPrintf(kERROR, "Can't create storage: %s\n", e.what());
    }

    m_callstack_unwinder.Initialize();

    m_cache_flush_timeout = chrono::seconds(2);
    try {
      const char *cache_flush_timeout_env = getenv("MEMHOOK_CACHE_FLUSH_INTERVAL");
      if (cache_flush_timeout_env) {
        ChronoDurationFromString(cache_flush_timeout_env, m_cache_flush_timeout);
      }
    } catch (const std::exception &e) {
      LogPrintf(kERROR, "Can't read cache flush timeout from environment: %s\n", e.what());
    }

    m_cache_flush_max_items = std::numeric_limits<std::size_t>::max();
    const char *cache_flush_max_items = getenv("MEMHOOK_CACHE_FLUSH_MAX_ITEMS");
    if (cache_flush_max_items) {
      m_cache_flush_max_items = strtoul(cache_flush_max_items, NULL, 10);
    }

    m_getprocinfo_policy = kFlushLocalCacheWhen;
    const char *getprocinfo_policy = getenv("MEMHOOK_GETPROCINFO_WHEN_STACK_UNWINDS");
    if (getprocinfo_policy && getprocinfo_policy[0] != '0') {
      m_getprocinfo_policy = kUnwindCallStackWhen;
    }

    m_cache_thread_runnable.Init(this, &Engine::FlushLocalCacheThread);
    m_cache_thread.Create(&m_cache_thread_runnable);
  }

  void Engine::OnDestroy() {
    m_cache_thread.Interrupt();
    m_cache_thread.Join();

    MutexLock lock(m_cache_mutex);
    FlushLocalCache(chrono::system_clock::now(), boost::chrono::seconds(0), true);
    m_storage.reset();
  }

  void Engine::DoHookAlloc(void *mem, size_t memsize) {
    if (BOOST_UNLIKELY(mem == NULL))
      return;

    const chrono::system_clock::time_point now = chrono::system_clock::now();

    CallStackInfo callstack;
    m_callstack_unwinder.GetCallStackInfo(
            callstack, 2, (m_getprocinfo_policy == kUnwindCallStackWhen));

    MutexLock lock(m_cache_mutex);
    std::pair<IndexedContainer::iterator, bool> status =
            m_cache.emplace(reinterpret_cast<uintptr_t>(mem), memsize, now);
    if (status.second) {
      const_cast<TraceInfo &>(*status.first).callstack.swap(callstack);
    }

    FlushLocalCache(now, m_cache_flush_timeout, false);
  }

  void Engine::DoHookFree(void *mem) {
    if (BOOST_UNLIKELY(mem == NULL))
      return;

    MutexLock lock(m_cache_mutex);
    IndexedContainer::iterator iter = m_cache.find(reinterpret_cast<uintptr_t>(mem));
    if (iter != m_cache.end()) {
      m_cache.erase(iter);
    } else if (m_storage) {
      lock.Unlock();
      m_storage->Remove(reinterpret_cast<uintptr_t>(mem));
    }
  }

  void Engine::DoHookRealloc(void *old_mem, void *new_mem, size_t new_size) {
    if (old_mem == new_mem) {
      DoHookUpdateSize(old_mem, new_size);
    } else {
      DoHookFree (old_mem);
      DoHookAlloc(new_mem, new_size);
    }
  }

  void Engine::DoHookUpdateSize(void *mem, size_t newsize) {
    if (BOOST_UNLIKELY(mem == NULL))
      return;

    MutexLock lock(m_cache_mutex);
    IndexedContainer::iterator iter = m_cache.find(reinterpret_cast<uintptr_t>(mem));
    if (iter != m_cache.end()) {
      const_cast<TraceInfo &>(*iter).memsize = newsize;
    } else if (m_storage) {
      m_storage->UpdateSize(reinterpret_cast<uintptr_t>(mem), newsize);
    }
  }

  unique_ptr<MappedStorage> Engine::NewStorage() const {
    const char *ipc_name = getenv("MEMHOOK_NET_HOST");
    if (ipc_name) {
      int ipc_port = MEMHOOK_NETWORK_STORAGE_PORT;
      const char *ipc_port_env = getenv("MEMHOOK_NET_PORT");
      if (ipc_port_env)
        ipc_port = strtoul(ipc_port_env, NULL, 10);
      return NewNetworkMappedStorage(ipc_name, ipc_port);
    }

    size_t ipc_size =
#ifdef __x86_64__
            (8ul << 30)  // default 8 Gb
#else
            (1ul << 30)  // default 1 Gb
#endif
            ;

    const char *ipc_size_env = getenv("MEMHOOK_SIZE_GB");
    if (ipc_size_env) {
      size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
      if (new_ipc_size != 0)
        ipc_size = (new_ipc_size << 30);
    } else if ((ipc_size_env = getenv("MEMHOOK_SIZE_MB"))) {
      size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
      if (new_ipc_size != 0)
        ipc_size = (new_ipc_size << 20);
    } else if ((ipc_size_env = getenv("MEMHOOK_SIZE_KB"))) {
      size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
      if (new_ipc_size != 0)
        ipc_size = (new_ipc_size << 10);
    } else if ((ipc_size_env = getenv("MEMHOOK_SIZE"))) {
      size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
      if (new_ipc_size != 0)
        ipc_size = new_ipc_size;
    }

    ipc_name = getenv("MEMHOOK_FILE");
    if (ipc_name)
      return NewMMFMappedStorage(ipc_name, ipc_size);

    ipc_name = getenv("MEMHOOK_SHM_NAME");
    if (!ipc_name)
      ipc_name = MEMHOOK_SHARED_MEMORY;

    return NewSHMMappedStorage(ipc_name, ipc_size);
  }

  void Engine::FlushLocalCache(const chrono::system_clock::time_point &now,
          const boost::chrono::seconds &timeout,
          bool dump_all) {
    typedef IndexedContainer::nth_index<1>::type Index1;
    Index1 &index1 = boost::get<1>(m_cache);

    const Index1::iterator end = dump_all ? index1.end() : index1.lower_bound(now - timeout);
    Index1::iterator iter = index1.begin();

    if (m_storage) {
      std::size_t n = 0;
      for (; iter != end && (dump_all || n < m_cache_flush_max_items); ++iter, ++n) {
        if (m_getprocinfo_policy == kFlushLocalCacheWhen)
          m_callstack_unwinder.GetCallStackUnwindProcInfo(const_cast<TraceInfo &>(*iter).callstack);
        m_storage->Add(iter->address, iter->memsize, iter->callstack, iter->timestamp);
      }

      if (n)
        m_storage->Flush();
    }

    if (iter != index1.begin())
      index1.erase(index1.begin(), iter);
  }

  void Engine::DoFlushCallStackCache() {
    m_callstack_unwinder.FlushCallStackCache();
  }

  void Engine::HookAlloc(void *mem, size_t memsize) MEMHOOK_NOEXCEPT {
    boost::intrusive_ptr<Engine> instance = GetInstance();
    if (BOOST_LIKELY(instance != NULL)) {
      instance->DoHookAlloc(mem, memsize);
    }
  }

  void Engine::HookFree(void *mem) MEMHOOK_NOEXCEPT {
    boost::intrusive_ptr<Engine> instance = GetInstance();
    if (BOOST_LIKELY(instance != NULL)) {
      instance->DoHookFree(mem);
    }
  }

  void Engine::HookRealloc(void *old_mem, void *new_mem, size_t new_size) MEMHOOK_NOEXCEPT {
    boost::intrusive_ptr<Engine> instance = GetInstance();
    if (BOOST_LIKELY(instance != NULL)) {
      instance->DoHookRealloc(old_mem, new_mem, new_size);
    }
  }

  void Engine::HookUpdateSize(void *mem, size_t newsize) MEMHOOK_NOEXCEPT {
    boost::intrusive_ptr<Engine> instance = GetInstance();
    if (BOOST_LIKELY(instance != NULL)) {
      instance->DoHookUpdateSize(mem, newsize);
    }
  }

  void Engine::FlushCallStackCache() MEMHOOK_NOEXCEPT {
    boost::intrusive_ptr<Engine> instance = GetInstance();
    if (BOOST_LIKELY(instance != NULL)) {
      instance->DoFlushCallStackCache();
    }
  }

  void *Engine::FlushLocalCacheThread() {
    NoHook no_hook;
    InterruptibleThread &thread = static_cast<InterruptibleThread &>(Thread::Current());
    const chrono::seconds double_cache_flush_timeout = m_cache_flush_timeout * 2;
    chrono::system_clock::time_point tp;
    do {

      tp = chrono::system_clock::now() + double_cache_flush_timeout;
      MutexLock lock(m_cache_mutex);
      FlushLocalCache(chrono::system_clock::now(), double_cache_flush_timeout, false);
      lock.Unlock();
    } while (thread.SleepUntil(tp));

    return NULL;
  }

}  // ns memhook
