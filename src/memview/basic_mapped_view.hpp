#ifndef MEMHOOK_BASIC_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/mapping_traits.hpp>
#include "mapped_view.hpp"
#include "mapped_view_base.hpp"
#include "interprocess_scoped_lock.hpp"
#include <boost/interprocess/creation_tags.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/noncopyable.hpp>
#include <iomanip>

namespace memhook {

template <typename Traits>
class basic_mapped_view : public mapped_view_base {
public:
    typedef typename Traits::segment segment_t;
    typedef mapped_container<Traits> mapped_container_t;
    typedef typename mapped_container_t::indexed_container_t indexed_container_t;
    typedef typename mapped_container_t::symbol_table_t symbol_table_t;

    explicit basic_mapped_view(const char *name);
    void write(std::ostream &os);

    std::size_t get_size();
    std::size_t get_free_memory();

    unique_char_buf_t get_module_path(uintptr_t addr) const;
    unique_char_buf_t get_proc_name(uintptr_t ip) const;

protected:
    virtual void do_write(std::ostream &os) = 0;

    indexed_container_t &get_indexed_container() {
        return container->indexed_container;
    }

private:
    segment_t segment;
    mapped_container_t *container;
};

template <typename Traits>
basic_mapped_view<Traits>::basic_mapped_view(const char *name)
    : segment(interprocess::open_only, name)
    , container(segment.template find<mapped_container_t>(MEMHOOK_SHARED_CONTAINER).first)
{}

template <typename Traits>
void basic_mapped_view<Traits>::write(std::ostream &os) {
    interprocess_scoped_lock lock(*container, no_lock());
    do_write(os);
}

template <typename Traits>
std::size_t basic_mapped_view<Traits>::get_size() {
    interprocess_scoped_lock lock(*container, no_lock());
    return segment.get_size();
}

template <typename Traits>
std::size_t basic_mapped_view<Traits>::get_free_memory() {
    interprocess_scoped_lock lock(*container, no_lock());
    return segment.get_free_memory();
}

template <typename Traits>
unique_char_buf_t basic_mapped_view<Traits>::get_module_path(uintptr_t shl_addr) const {
    typename symbol_table_t::const_iterator iter = container->shltab.find(shl_addr);
    if (iter != container->shltab.end())
        return unique_char_buf_t(iter->second.c_str(), unique_char_buf_null_free);
    return unique_char_buf_t("<unknown>", unique_char_buf_null_free);
}

template <typename Traits>
unique_char_buf_t basic_mapped_view<Traits>::get_proc_name(uintptr_t ip) const {
    typename symbol_table_t::const_iterator iter = container->symtab.find(ip);
    if (iter != container->symtab.end())
        return unique_char_buf_t(iter->second.c_str(), unique_char_buf_null_free);
    return unique_char_buf_t("<unknown>", unique_char_buf_null_free);
}

} // memhook

#endif // MEMHOOK_BASIC_MAPPED_VIEW_HPP_INCLUDED
