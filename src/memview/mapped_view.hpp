#ifndef MEMHOOK_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_MAPPED_VIEW_HPP_INCLUDED

#include <memhook/common.hpp>
#include <boost/noncopyable.hpp>
#include <boost/move/unique_ptr.hpp>
#include <iosfwd>

namespace memhook {

struct mapped_view_req : private noncopyable {
    virtual ~mapped_view_req() {}
};

struct mapped_view : private noncopyable {
    enum {
        no_lock         = 0x0001,
        show_callstack  = 0x0002,
        sort_by_address = 0x0010,
        sort_by_time    = 0x0020,
        sort_by_size    = 0x0040,
    };

    virtual ~mapped_view() {}
    virtual void write(std::ostream &os) = 0;
    virtual void add_req(mapped_view_req *req) = 0;
    virtual void set_option(uint32_t option, bool setval = true) = 0;

    virtual std::size_t get_size() = 0;
    virtual std::size_t get_free_memory() = 0;
};

struct mapped_view_kit : private noncopyable {
    virtual ~mapped_view_kit() {}
    virtual mapped_view     *make_view(const char *name) = 0;
    virtual mapped_view     *make_aggregated_view(const char *name) = 0;
    virtual mapped_view_req *make_min_size_req(std::size_t minsize) = 0;
    virtual mapped_view_req *make_min_time_req(const system_clock_t::time_point &min_time) = 0;
    virtual mapped_view_req *make_max_time_req(const system_clock_t::time_point &max_time) = 0;
    virtual mapped_view_req *make_min_time_from_now_req(const system_clock_t::duration &min_time) = 0;
    virtual mapped_view_req *make_max_time_from_now_req(const system_clock_t::duration &max_time) = 0;
};

movelib::unique_ptr<mapped_view_kit> make_shm_view_kit();
movelib::unique_ptr<mapped_view_kit> make_mmf_view_kit();

} // memhook

#endif // MEMHOOK_MAPPED_VIEW_HPP_INCLUDED
