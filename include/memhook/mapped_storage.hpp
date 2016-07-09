#ifndef MEMHOOK_MAPPED_STORAGE_HPP_INCLUDED
#define MEMHOOK_MAPPED_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/callstack.hpp>
#include <boost/noncopyable.hpp>
#include <boost/chrono/system_clocks.hpp>

namespace memhook {

struct mapped_storage : private boost::noncopyable {
    virtual ~mapped_storage() {}
    virtual void insert(uintptr_t address, std::size_t memsize,
        callstack_container &callstack) = 0;
    virtual void insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        callstack_container &callstack) = 0;
    virtual bool erase(uintptr_t address) = 0;
    virtual bool update_size(uintptr_t address, std::size_t memsize) = 0;
};

mapped_storage *make_shared_memory_storage(const char *name, std::size_t size);
mapped_storage *make_mapped_file_storage(const char *name, std::size_t size);

} // memhook

#endif // MEMHOOK_MAPPED_STORAGE_HPP_INCLUDED
