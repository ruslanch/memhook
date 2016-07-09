#ifndef MEMHOOK_MAPPED_VIEW_BASE_HPP_INCLUDED
#define MEMHOOK_MAPPED_VIEW_BASE_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/mapping_traits.hpp>
#include "mapped_view.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/move/unique_ptr.hpp>

namespace memhook {

typedef boost::movelib::unique_ptr<const char, void (*)(const char *)> unique_char_buf_t;
void unique_char_buf_null_free(const char *) BOOST_NOEXCEPT_OR_NOTHROW;
void unique_char_buf_full_free(const char *) BOOST_NOEXCEPT_OR_NOTHROW;
unique_char_buf_t cxx_symbol_demangle(const char *source);

class mapped_view_base : public mapped_view {
    boost::ptr_vector<mapped_view_req> reqs_;
    uint32_t options_;

    enum {
        opt_no_lock         = 0x0001,
        opt_show_callstack  = 0x0002,
        opt_sort_by_address = 0x0010,
        opt_sort_by_time    = 0x0020,
        opt_sort_by_size    = 0x0040,
    };

    bool get_option(uint32_t option) const;
    void set_option(uint32_t option, bool setval);

public:
    mapped_view_base();

    bool no_lock() const            { return get_option(opt_no_lock);         }
    void no_lock(bool flag)         { set_option(opt_no_lock, flag);          }
    bool show_callstack() const     { return get_option(opt_show_callstack);  }
    void show_callstack(bool flag)  { set_option(opt_show_callstack, flag);       }
    bool sort_by_address() const    { return get_option(opt_sort_by_address); }
    void sort_by_address(bool flag) { set_option(opt_sort_by_address, flag);  }
    bool sort_by_time() const       { return get_option(opt_sort_by_time);    }
    void sort_by_time(bool flag)    { set_option(opt_sort_by_time, flag);     }
    bool sort_by_size() const       { return get_option(opt_sort_by_size);    }
    void sort_by_size(bool flag)    { set_option(opt_sort_by_size, flag);     }

    void add_req(mapped_view_req *req);
    bool check_traceinfo_reqs(const traceinfo_base &tinfo) const;

    virtual unique_char_buf_t get_module_path(uintptr_t shl_addr) const = 0;
    virtual unique_char_buf_t get_proc_name(uintptr_t ip) const = 0;

    static bool is_interrupted();
};

} // memhook

#endif // MEMHOOK_MAPPED_VIEW_BASE_HPP_INCLUDED

