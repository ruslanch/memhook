#include "engine.h"
#include "utils.h"

#include <memhook/chrono_utils.h>
#include <memhook/mapped_storage_creator.h>

#include <boost/foreach.hpp>

namespace memhook
{

extern unique_ptr<MappedStorage> NewNetworkStorage(const char *host, int port);

namespace
{
    struct UpdateCallStack
    {
        CallStackInfo &callstack;

        explicit UpdateCallStack(CallStackInfo &callstack)
            : callstack(callstack)
        {}

        void operator()(Engine::TraceInfo &traceinfo)
        {
            traceinfo.callstack.swap(callstack);
        }
    };

    struct UpdateMemSize
    {
        std::size_t newsize;

        explicit UpdateMemSize(std::size_t newsize)
            : newsize(newsize)
        {}

        void operator()(Engine::TraceInfo &traceinfo) const
        {
            traceinfo.memsize = newsize;
        }
    };
} // ns

void Engine::OnInitialize()
{
    try
    {
        unique_ptr<MappedStorage> storage(NewStorage());
        if (storage)
            storage_.swap(storage);
    }
    catch (const std::exception &e)
    {
        PrintErrorMessage("Can't create storage: ", e.what());
    }

    callstack_unwinder_.Initialize();

    cache_flush_timeout_ = boost::chrono::seconds(2);
    try
    {
        const char *cache_flush_timeout_env = getenv("MEMHOOK_CACHE_FLUSH_TIMEOUT");
        if (cache_flush_timeout_env)
        {
            ChronoDurationFromString(cache_flush_timeout_env, cache_flush_timeout_);
        }
    }
    catch (const std::exception &e)
    {
        PrintErrorMessage("Can't read cache flush timeout from environment: ", e.what());
    }
}

void Engine::OnDestroy()
{
    FlushLocalCache(boost::chrono::system_clock::now(), boost::chrono::seconds(0));
    storage_.reset();
}

void Engine::Insert(void *ptr, size_t memsize)
{
    const boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();

    CallStackInfo callstack;
    callstack_unwinder_.GetCallStackInfo(callstack, 2);

    boost::unique_lock<boost::mutex> lock(cache_mutex_);
    std::pair<IndexedContainer::iterator, bool> status =
        cache_.emplace(reinterpret_cast<uintptr_t>(ptr), memsize, now);
    if (status.second)
    {
        cache_.modify(status.first, UpdateCallStack(callstack));
    }

    FlushLocalCache(now, cache_flush_timeout_);
}

void Engine::Erase(void *ptr)
{
    boost::unique_lock<boost::mutex> lock(cache_mutex_);
    IndexedContainer::iterator iter = cache_.find(reinterpret_cast<uintptr_t>(ptr));
    if (iter != cache_.end())
    {
        cache_.erase(iter);
    }
    else if (storage_)
    {
        lock.unlock();
        storage_->Erase(reinterpret_cast<uintptr_t>(ptr));
    }
}

void Engine::UpdateSize(void *ptr, size_t newsize)
{
    boost::unique_lock<boost::mutex> lock(cache_mutex_);
    IndexedContainer::iterator iter = cache_.find(reinterpret_cast<uintptr_t>(ptr));
    if (iter != cache_.end())
    {
        cache_.modify(iter, UpdateMemSize(newsize));
    }
    else if (storage_)
    {
        storage_->UpdateSize(reinterpret_cast<uintptr_t>(ptr), newsize);
    }
}

unique_ptr<MappedStorage> Engine::NewStorage() const
{
    const char *ipc_name = getenv("MEMHOOK_NET_HOST");
    if (ipc_name)
    {
        int ipc_port = MEMHOOK_NETWORK_STORAGE_PORT;
        const char *ipc_port_env = getenv("MEMHOOK_NET_PORT");
        if (ipc_port_env)
            ipc_port = strtoul(ipc_port_env, NULL, 10);
        return NewNetworkMappedStorage(ipc_name, ipc_port);
    }

    size_t ipc_size = (8ul << 30); // default 8 Gb
    const char *ipc_size_env = getenv("MEMHOOK_SIZE_GB");
    if (ipc_size_env)
    {
        size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
        if (new_ipc_size != 0)
            ipc_size = (new_ipc_size << 30);
    }
    else if ((ipc_size_env = getenv("MEMHOOK_SIZE_MB")))
    {
        size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
        if (new_ipc_size != 0)
            ipc_size = (new_ipc_size << 20);
    }
    else if ((ipc_size_env = getenv("MEMHOOK_SIZE_KB")))
    {
        size_t new_ipc_size = strtoul(ipc_size_env, NULL, 10);
        if (new_ipc_size != 0)
            ipc_size = (new_ipc_size << 10);
    }
    else if ((ipc_size_env = getenv("MEMHOOK_SIZE")))
    {
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

void Engine::FlushLocalCache(const boost::chrono::system_clock::time_point &now,
        const boost::chrono::seconds &timeout)
{
    typedef typename IndexedContainer::template nth_index<1>::type Index1;
    Index1 &index1 = boost::get<1>(cache_);

    const typename Index1::iterator iter = index1.lower_bound(now - timeout);

    if (storage_)
    {
        std::size_t n = 0;
        BOOST_FOREACH(const TraceInfo &traceinfo, std::make_pair(index1.begin(), iter))
        {
            ++n;
            storage_->Insert(traceinfo.address, traceinfo.memsize,
                traceinfo.timestamp, traceinfo.callstack);
        }

        if (n)
        {
            storage_->Flush();
        }
    }

    if (iter != index1.begin())
    {
        index1.erase(index1.begin(), iter);
    }
}

void Engine::FlushCallStackCache()
{
    callstack_unwinder_.FlushCallStackCache();
}

} // ns memhook
