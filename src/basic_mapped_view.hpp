#ifndef MEMHOOK_BASIC_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_HPP_INCLUDED

#include "common.hpp"
#include "mapping_traits.hpp"
#include "mapped_view.hpp"
#include "basic_mapped_view_req.hpp"
#include "interprocess_scoped_lock.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scope_exit.hpp>
#include <iomanip>

namespace memhook {
namespace mapped_view_detail {
const char unknown_tag[] = "<unknown>";
bool is_interrupted();
} // mapped_view_detail

struct basic_mapped_view_base : mapped_view {
protected:
    typedef movelib::unique_ptr<const char, void (*)(const char *)> unique_char_buf_t;
    unique_char_buf_t cxa_demangle(const char *source) BOOST_NOEXCEPT_OR_NOTHROW;
};

template <typename Traits>
struct basic_mapped_view : basic_mapped_view_base {
    typedef traceinfo<Traits>             traceinfo_t;
    typedef basic_mapped_view_req<Traits> mapped_view_req_t;

    typedef typename Traits::segment segment_t;
    segment_t segment;

    typedef mapped_container<Traits>                         mapped_container_t;
    typedef typename mapped_container_t::indexed_container_t indexed_container_t;
    typedef typename mapped_container_t::symbol_table_t      symbol_table_t;
    mapped_container_t *container;

    ptr_vector<mapped_view_req_t> reqs_;
    uint32_t options_;

    explicit basic_mapped_view(const char *name);
    void write(std::ostream &os);
    void add_req(mapped_view_req *req);
    void set_option(uint32_t option, bool setval);
    std::size_t get_size();
    std::size_t get_free_memory();

protected:
    virtual void do_write(std::ostream &os) = 0;

    bool is_no_lock()         const { return options_ & (uint32_t)no_lock;         }
    bool is_show_callstack()  const { return options_ & (uint32_t)show_callstack;  }
    bool is_sort_by_address() const { return options_ & (uint32_t)sort_by_address; }
    bool is_sort_by_size()    const { return options_ & (uint32_t)sort_by_size;    }
    bool is_sort_by_time()    const { return options_ & (uint32_t)sort_by_time;    }

    const char *resolve_shl_path(uintptr_t shl_addr) const;
    const char *resolve_procname(uintptr_t ip) const;
};

template <typename Traits>
basic_mapped_view<Traits>::basic_mapped_view(const char *name)
    : segment(interprocess::open_only, name)
    , container(segment.template find<mapped_container_t>(MEMHOOK_SHARED_CONTAINER).first)
    , reqs_()
    , options_(sort_by_time)
{}

template <typename Traits>
void basic_mapped_view<Traits>::write(std::ostream &os) {
    interprocess_scoped_lock lock(*container, is_no_lock());
    do_write(os);
}

template <typename Traits>
void basic_mapped_view<Traits>::add_req(mapped_view_req *req) {
    reqs_.push_back(static_cast<mapped_view_req_t *>(req));
}

template <typename Traits>
void basic_mapped_view<Traits>::set_option(uint32_t option, bool setval) {
    const uint32_t sortmask = 0x00f0;
    if (option & sortmask)
        options_ &= ~sortmask;

    if (setval)
        options_ |= option;
    else
        options_ &= ~option;
}

template <typename Traits>
std::size_t basic_mapped_view<Traits>::get_size() {
    interprocess_scoped_lock lock(*container, is_no_lock());
    return segment.get_size();
}

template <typename Traits>
std::size_t basic_mapped_view<Traits>::get_free_memory() {
    interprocess_scoped_lock lock(*container, is_no_lock());
    return segment.get_free_memory();
}

template <typename Traits>
const char *basic_mapped_view<Traits>::resolve_shl_path(uintptr_t shl_addr) const {
    typename symbol_table_t::const_iterator iter = container->shltab.find(shl_addr);
    if (iter != container->shltab.end()) {
        return iter->second.c_str();
    }
    return mapped_view_detail::unknown_tag;
}

template <typename Traits>
const char *basic_mapped_view<Traits>::resolve_procname(uintptr_t ip) const {
    typename symbol_table_t::const_iterator iter = container->symtab.find(ip);
    if (iter != container->symtab.end()) {
        return iter->second.c_str();
    }
    return mapped_view_detail::unknown_tag;
}

} // memhook

#endif // MEMHOOK_BASIC_MAPPED_VIEW_HPP_INCLUDED
