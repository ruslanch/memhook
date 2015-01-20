#ifndef MEMHOOK_BASIC_MAPPED_STORAGE_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_STORAGE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/mapping_traits.hpp>
#include <memhook/scoped_signal.hpp>
#include "mapped_storage.hpp"
#include <boost/algorithm/string.hpp>

namespace memhook {
namespace {
    chrono::system_clock::time_point system_clock_now() BOOST_NOEXCEPT {
        timespec ts = {0};
        clock_gettime(CLOCK_REALTIME, &ts);
        return system_clock_t::time_point(system_clock_t::duration(
            static_cast<system_clock_t::rep>(ts.tv_sec) * 1000000000 + ts.tv_nsec));
    }
} // namespace

template <typename Traits>
struct basic_mapped_storage : mapped_storage {
    typedef mapping_cleaner<Traits> cleaner_t;
    cleaner_t cleaner;

    typedef typename Traits::segment segment_t;
    segment_t segment;

    typedef typename Traits::generic_allocator generic_allocator_t;
    generic_allocator_t allocator_instance;

    typedef mapped_container<Traits> mapped_container_t;
    mapped_container_t *container;

    typedef typename mapped_container_t::indexed_container_t    indexed_container_t;
    typedef typename mapped_container_t::symbol_table_t         symbol_table_t;
    typedef typename mapped_container_t::string_type            string_type;

    typedef traceinfo<Traits> traceinfo_t;
    typedef typename traceinfo_t::traceinfo_callstack_container traceinfo_callstack_container_t;

    basic_mapped_storage(const char *name, std::size_t size)
        : cleaner(name)
        , segment(interprocess::create_only, name, size)
        , allocator_instance(segment.get_segment_manager())
        , container(segment.template construct<mapped_container_t>(
            MEMHOOK_SHARED_CONTAINER)(allocator_instance))
    {}

    void insert(uintptr_t address, std::size_t memsize, const callstack_container &callstack);
    bool erase(uintptr_t address);
    bool update_size(uintptr_t address, std::size_t memsize);

private:
    struct symbol_table_updater {
        mapped_container_t  *const container;
        generic_allocator_t &allocator_instance;
        explicit symbol_table_updater(mapped_container_t *container,
                generic_allocator_t &allocator_instance)
            : container(container)
            , allocator_instance(allocator_instance)
        {}

        bool operator()(const callstack_record &r) {
            typename symbol_table_t::const_iterator iter = container->symtab.find(r.ip);
            if (iter == container->symtab.end()) {
                string_type symbol(r.procname.begin(), r.procname.end(), allocator_instance);
                container->symtab.emplace(r.ip, move(symbol));

                iter = container->shltab.find(r.shl_addr);
                if (iter == container->shltab.end()) {
                    string_type shl_path(r.shl_path.begin(), r.shl_path.end(),
                        allocator_instance);
                    container->shltab.emplace(r.shl_addr, move(shl_path));
                }
            }
            return false;
        }
    };

    struct field_memsize_setter {
        std::size_t memsize;
        explicit field_memsize_setter(std::size_t memsize) : memsize(memsize) {}
        void operator()(traceinfo_t &tinfo) const {
            tinfo.memsize = memsize;
        }
    };
};

template <typename Traits>
void basic_mapped_storage<Traits>::insert(uintptr_t address, std::size_t memsize,
        const callstack_container &callstack) {
    system_clock_t::time_point time_point = system_clock_now();
    scoped_signal_block signal_block;
    interprocess::scoped_lock<interprocess::interprocess_mutex> lock(*container);
    if (find_if(callstack, symbol_table_updater(container, allocator_instance)) != callstack.end())
        return;
     container->indexed_container.emplace(address, memsize, time_point, callstack,
        allocator_instance);
}

template <typename Traits>
bool basic_mapped_storage<Traits>::erase(uintptr_t address) {
    scoped_signal_block signal_block;
    interprocess::scoped_lock<interprocess::interprocess_mutex> lock(*container);
    typedef typename indexed_container_t::template nth_index<0>::type index0;
    index0 &idx = get<0>(container->indexed_container);
    const typename index0::iterator iter = idx.find(address);
    const bool ret = iter != idx.end();
    if (ret)
        idx.erase(iter);
    return ret;
}

template <typename Traits>
bool basic_mapped_storage<Traits>::update_size(uintptr_t address, std::size_t memsize) {
    scoped_signal_block signal_block;
    interprocess::scoped_lock<interprocess::interprocess_mutex> lock(*container);
    typedef typename indexed_container_t::template nth_index<0>::type index0;
    index0 &idx = get<0>(container->indexed_container);
    const typename index0::iterator iter = idx.find(address);
    const bool ret = iter != idx.end();
    if (ret)
        idx.modify(iter, field_memsize_setter(memsize));
    return ret;
}

} // memhook

#endif // MEMHOOK_BASIC_MAPPED_STORAGE_HPP_INCLUDED
