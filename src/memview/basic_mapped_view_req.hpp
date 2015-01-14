#ifndef MEMHOOK_BASIC_MAPPED_VIEW_REQ_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_REQ_HPP_INCLUDED

#include <memhook/common.hpp>
#include <memhook/mapping_traits.hpp>
#include "mapped_view.hpp"

namespace memhook {

template <typename Traits>
struct basic_mapped_view_req : mapped_view_req {
    typedef traceinfo<Traits> traceinfo_t;
    virtual bool invoke(const traceinfo_t &tinfo) const = 0;
};

template <typename Traits>
struct basic_min_size_mapped_view_req : basic_mapped_view_req<Traits> {
    typedef traceinfo<Traits> traceinfo_t;
    std::size_t min_size;
    basic_min_size_mapped_view_req(std::size_t min_size) : min_size(min_size) {}
    bool invoke(const traceinfo_t &tinfo) const {
        return tinfo.memsize >= min_size;
    }
};

template <typename Traits>
struct basic_min_time_mapped_view_req : basic_mapped_view_req<Traits> {
    typedef traceinfo<Traits> traceinfo_t;
    system_clock_t::time_point min_time;
    basic_min_time_mapped_view_req(const system_clock_t::time_point &min_time) : min_time(min_time) {}
    bool invoke(const traceinfo_t &tinfo) const {
        return tinfo.timestamp >= min_time;
    }
};

template <typename Traits>
struct basic_max_time_mapped_view_req : basic_mapped_view_req<Traits> {
    typedef traceinfo<Traits> traceinfo_t;
    system_clock_t::time_point max_time;
    basic_max_time_mapped_view_req(const system_clock_t::time_point &max_time) : max_time(max_time) {}
    bool invoke(const traceinfo_t &tinfo) const {
        return tinfo.timestamp <= max_time;
    }
};

template <typename Traits>
struct basic_min_time_from_now_mapped_view_req : basic_mapped_view_req<Traits> {
    typedef traceinfo<Traits> traceinfo_t;
    system_clock_t::time_point current_time;
    system_clock_t::duration   min_duration;
    basic_min_time_from_now_mapped_view_req(const system_clock_t::duration &min_duration)
        : current_time(system_clock_t::now())
        , min_duration(min_duration) {}
    bool invoke(const traceinfo_t &tinfo) const {
        return (current_time - tinfo.timestamp) >= min_duration;
    }
};

template <typename Traits>
struct basic_max_time_from_now_mapped_view_req : basic_mapped_view_req<Traits> {
    typedef traceinfo<Traits> traceinfo_t;
    system_clock_t::time_point current_time;
    system_clock_t::duration   max_duration;
    basic_max_time_from_now_mapped_view_req(const system_clock_t::duration &max_duration)
        : current_time(system_clock_t::now())
        , max_duration(max_duration) {}
    bool invoke(const traceinfo_t &tinfo) const {
        return (current_time - tinfo.timestamp) <= max_duration;
    }
};

} // namespace

#endif // MEMHOOK_BASIC_MAPPED_VIEW_REQ_HPP_INCLUDED
