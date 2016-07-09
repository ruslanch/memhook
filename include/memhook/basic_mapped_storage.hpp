#ifndef MEMHOOK_BASIC_MAPPED_STORAGE_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_STORAGE_HPP_INCLUDED

#include "common.hpp"
#include "mapping_traits.hpp"
#include "scoped_signal.hpp"
#include "mapped_storage.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace memhook {

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

    typedef mapped_traceinfo<Traits> traceinfo_t;

    basic_mapped_storage(const char *name, std::size_t size)
        : cleaner(name)
        , segment(boost::interprocess::create_only, name, size)
        , allocator_instance(segment.get_segment_manager())
        , container(segment.template construct<mapped_container_t>(
            MEMHOOK_SHARED_CONTAINER)(allocator_instance))
    {}

    void insert(uintptr_t address, std::size_t memsize,
        callstack_container &callstack);
    void insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        callstack_container &callstack);
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
                container->symtab.emplace(r.ip, boost::move(symbol));

                iter = container->shltab.find(r.shl_addr);
                if (iter == container->shltab.end()) {
                    string_type shl_path(r.shl_path.begin(), r.shl_path.end(),
                        allocator_instance);
                    container->shltab.emplace(r.shl_addr, boost::move(shl_path));
                }
            }
            return false;
        }
    };
};

template <typename Traits>
void basic_mapped_storage<Traits>::insert(uintptr_t address, std::size_t memsize,
        callstack_container &callstack) {
    insert(address, memsize, boost::chrono::system_clock::now(), callstack);
}


template <typename Traits>
void basic_mapped_storage<Traits>::insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        callstack_container &callstack) {
    scoped_signal_block signal_block;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    if (boost::find_if(callstack, symbol_table_updater(container, allocator_instance)) != callstack.end())
        return;
     container->indexed_container.emplace(address, memsize, timestamp, callstack,
        allocator_instance);
}

template <typename Traits>
bool basic_mapped_storage<Traits>::erase(uintptr_t address) {
    scoped_signal_block signal_block;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    typedef typename indexed_container_t::template nth_index<0>::type index0;
    index0 &idx = boost::get<0>(container->indexed_container);
    const typename index0::iterator iter = idx.find(address);
    const bool ret = iter != idx.end();
    if (ret)
        idx.erase(iter);
    return ret;
}

template <typename Traits>
bool basic_mapped_storage<Traits>::update_size(uintptr_t address, std::size_t memsize) {
    scoped_signal_block signal_block;
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*container);
    typedef typename indexed_container_t::template nth_index<0>::type index0;
    index0 &idx = boost::get<0>(container->indexed_container);
    const typename index0::iterator iter = idx.find(address);
    const bool ret = iter != idx.end();
    if (ret)
        idx.modify(iter, boost::lambda::bind(&traceinfo_base::memsize, boost::lambda::_1) = memsize);
    return ret;
}

} // memhook

#endif // MEMHOOK_BASIC_MAPPED_STORAGE_HPP_INCLUDED
