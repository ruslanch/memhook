#ifndef MEMHOOK_MAPPED_VIEW_REQ_HPP_INCLUDED
#define MEMHOOK_MAPPED_VIEW_REQ_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/mapping_traits.hpp>
#include "mapped_view.hpp"

namespace memhook {

class mapped_view_req : noncopyable {
public:
    virtual ~mapped_view_req() {}
    virtual bool invoke(const traceinfo_base &tinfo) const = 0;
};

class min_size_mapped_view_req : public mapped_view_req {
    std::size_t min_size;
public:
    min_size_mapped_view_req(std::size_t min_size) : min_size(min_size) {}
    bool invoke(const traceinfo_base &tinfo) const {
        return tinfo.memsize() >= min_size;
    }
};

class min_time_mapped_view_req : public mapped_view_req {
    system_clock_t::time_point min_time;
public:
    min_time_mapped_view_req(const system_clock_t::time_point &min_time) : min_time(min_time) {}
    bool invoke(const traceinfo_base &tinfo) const {
        return tinfo.timestamp() >= min_time;
    }
};

class max_time_mapped_view_req : public mapped_view_req {
    system_clock_t::time_point max_time;
public:
    max_time_mapped_view_req(const system_clock_t::time_point &max_time) : max_time(max_time) {}
    bool invoke(const traceinfo_base &tinfo) const {
        return tinfo.timestamp() <= max_time;
    }
};

class min_time_from_now_mapped_view_req : public mapped_view_req {
    system_clock_t::time_point current_time;
    system_clock_t::duration   min_duration;
public:
    min_time_from_now_mapped_view_req(const system_clock_t::duration &min_duration)
            : current_time(system_clock_t::now())
            , min_duration(min_duration) {}
    bool invoke(const traceinfo_base &tinfo) const {
        return (current_time - tinfo.timestamp()) >= min_duration;
    }
};

class max_time_from_now_mapped_view_req : public mapped_view_req {
    system_clock_t::time_point current_time;
    system_clock_t::duration   max_duration;
public:
    max_time_from_now_mapped_view_req(const system_clock_t::duration &max_duration)
            : current_time(system_clock_t::now())
            , max_duration(max_duration) {}
    bool invoke(const traceinfo_base &tinfo) const {
        return (current_time - tinfo.timestamp()) <= max_duration;
    }
};

} // namespace

#endif // MEMHOOK_MAPPED_VIEW_REQ_HPP_INCLUDED
