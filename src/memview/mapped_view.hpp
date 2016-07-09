#ifndef MEMHOOK_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_MAPPED_VIEW_HPP_INCLUDED

#include <memhook/common.hpp>
#include <boost/noncopyable.hpp>
#include <boost/move/unique_ptr.hpp>
#include <iosfwd>

namespace memhook {

class mapped_view_req;

class mapped_view : boost::noncopyable {
public:
    virtual ~mapped_view() {}
    virtual void write(std::ostream &os)    = 0;
    virtual void add_req(mapped_view_req *req) = 0;
    virtual bool no_lock() const            = 0;
    virtual void no_lock(bool flag)         = 0;
    virtual bool show_callstack() const     = 0;
    virtual void show_callstack(bool flag)  = 0;
    virtual bool sort_by_address() const    = 0;
    virtual void sort_by_address(bool flag) = 0;
    virtual bool sort_by_time() const       = 0;
    virtual void sort_by_time(bool flag)    = 0;
    virtual bool sort_by_size() const       = 0;
    virtual void sort_by_size(bool flag)    = 0;
    virtual std::size_t get_size()          = 0;
    virtual std::size_t get_free_memory()   = 0;
};

class mapped_view_kit : boost::noncopyable {
public:
    virtual ~mapped_view_kit() {}
    virtual mapped_view     *make_view(const char *name) = 0;
    virtual mapped_view     *make_aggregated_view(const char *name) = 0;
    virtual mapped_view_req *make_min_size(std::size_t minsize) = 0;
    virtual mapped_view_req *make_min_time(const boost::chrono::system_clock::time_point &min_time) = 0;
    virtual mapped_view_req *make_max_time(const boost::chrono::system_clock::time_point &max_time) = 0;
    virtual mapped_view_req *make_min_time_from_now(const boost::chrono::system_clock::duration &min_time) = 0;
    virtual mapped_view_req *make_max_time_from_now(const boost::chrono::system_clock::duration &max_time) = 0;
};

boost::movelib::unique_ptr<mapped_view_kit> make_shm_view_kit();
boost::movelib::unique_ptr<mapped_view_kit> make_mmf_view_kit();

} // memhook

#endif // MEMHOOK_MAPPED_VIEW_HPP_INCLUDED
