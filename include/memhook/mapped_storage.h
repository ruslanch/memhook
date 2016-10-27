#ifndef MEMHOOK_INCLUDE_MAPPED_STORAGE_H_INCLUDED
#define MEMHOOK_INCLUDE_MAPPED_STORAGE_H_INCLUDED

#include <memhook/common.h>
#include <memhook/callstack.h>
#include <memhook/traceinfo.h>

#include <boost/noncopyable.hpp>
#include <boost/chrono/system_clocks.hpp>

namespace memhook
{

struct MappedStorage : noncopyable
{
    virtual ~MappedStorage() {}
    virtual void Insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        const CallStackInfo &callstack) = 0;
    virtual bool Erase(uintptr_t address) = 0;
    virtual bool UpdateSize(uintptr_t address, std::size_t memsize) = 0;
    virtual void Clear() = 0;
    virtual void Flush() = 0;
    virtual std::string GetName() const = 0;
};

unique_ptr<MappedStorage> NewSHMMappedStorage(const char *path, std::size_t size);
unique_ptr<MappedStorage> NewMMFMappedStorage(const char *path, std::size_t size);
unique_ptr<MappedStorage> NewNetworkMappedStorage(const char *host, int port);
unique_ptr<MappedStorage> NewLevelDBMappedStorage(const char *path);

} // memhook

#endif
