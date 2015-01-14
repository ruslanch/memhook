#ifndef MEMHOOK_MAPPED_STORAGE_HPP_INCLUDED
#define MEMHOOK_MAPPED_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/callstack.hpp>
#include <boost/noncopyable.hpp>

namespace memhook {

struct mapped_storage : private noncopyable {
    virtual ~mapped_storage() {}
    virtual void insert(uintptr_t address, std::size_t memsize,
        const callstack_container &callstack) = 0;
    virtual bool erase(uintptr_t address) = 0;
    virtual bool update_size(uintptr_t address, std::size_t memsize) = 0;
};

mapped_storage *make_shm_storage(const char *name, std::size_t size);
mapped_storage *make_mmf_storage(const char *name, std::size_t size);

} // memhook

#endif // MEMHOOK_MAPPED_STORAGE_HPP_INCLUDED
