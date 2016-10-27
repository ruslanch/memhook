#ifndef MEMHOOK_SRC_MEMHOOK_ENGINE_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_ENGINE_H_INCLUDED

#include "common.h"
#include "singleton.h"
#include "thread.h"
#include "callstack_unwinder.h"

#include <memhook/callstack.h>
#include <memhook/traceinfo.h>
#include <memhook/mapped_storage.h>

#include <boost/move/unique_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/thread/thread.hpp>

namespace memhook
{

class Engine : public SingletonImpl<Engine>
{
public:
    struct TraceInfo : TraceInfoBase
    {
        CallStackInfo callstack;

        TraceInfo(uintptr_t address, size_t memsize,
                const boost::chrono::system_clock::time_point &timestamp)
            : TraceInfoBase(address, memsize, timestamp)
            , callstack()
        {}
    };

    void OnInitialize();
    void OnDestroy();

    void Insert(void *ptr, size_t memsize);
    void Erase(void *ptr);
    void UpdateSize(void *ptr, size_t newsize);
    void FlushCallStackCache();

private:
    boost::movelib::unique_ptr<MappedStorage> NewStorage() const;
    void FlushLocalCache(const boost::chrono::system_clock::time_point &now,
        const boost::chrono::seconds &timeout);

    boost::movelib::unique_ptr<MappedStorage> storage_;

    typedef boost::multi_index_container<
        TraceInfo,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::member<TraceInfoBase,
                    uintptr_t, &TraceInfoBase::address
                >
            >,
            boost::multi_index::ordered_non_unique<
                boost::multi_index::member<TraceInfoBase,
                    boost::chrono::system_clock::time_point, &TraceInfoBase::timestamp
                >
            >
        >
    > IndexedContainer;

    Mutex            cache_mutex_;
    IndexedContainer cache_;

    CallStackUnwinder callstack_unwinder_;

    boost::chrono::seconds cache_flush_timeout_;
};

} // ns memhook

#endif
