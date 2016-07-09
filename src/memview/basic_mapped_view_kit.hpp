#ifndef MEMHOOK_BASIC_MAPPED_VIEW_KIT_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_KIT_HPP_INCLUDED

#include "basic_mapped_view.hpp"
#include "basic_simple_mapped_view.hpp"
#include "basic_aggregated_mapped_view.hpp"
#include "mapped_view_req.hpp"


namespace memhook {

template <typename Traits>
struct basic_mapped_view_kit : mapped_view_kit {
    mapped_view     *make_view(const char *name);
    mapped_view     *make_aggregated_view(const char *name);
    mapped_view_req *make_min_size(std::size_t minsize);
    mapped_view_req *make_min_time(const boost::chrono::system_clock::time_point &min_time);
    mapped_view_req *make_max_time(const boost::chrono::system_clock::time_point &max_time);
    mapped_view_req *make_min_time_from_now(const boost::chrono::system_clock::duration &min_time);
    mapped_view_req *make_max_time_from_now(const boost::chrono::system_clock::duration &max_time);
};

template <typename Traits>
mapped_view *basic_mapped_view_kit<Traits>::make_view(const char *name) {
    return new basic_simple_mapped_view<Traits>(name);
}

template <typename Traits>
mapped_view *basic_mapped_view_kit<Traits>::make_aggregated_view(const char *name) {
    return new basic_aggregated_mapped_view<Traits>(name);
}

template <typename Traits>
mapped_view_req *basic_mapped_view_kit<Traits>::make_min_size(std::size_t minsize) {
    return new min_size_mapped_view_req(minsize);
}

template <typename Traits>
mapped_view_req *basic_mapped_view_kit<Traits>::make_min_time(
        const boost::chrono::system_clock::time_point &min_time) {
    return new min_time_mapped_view_req(min_time);
}

template <typename Traits>
mapped_view_req *basic_mapped_view_kit<Traits>::make_max_time(
        const boost::chrono::system_clock::time_point &max_time) {
    return new max_time_mapped_view_req(max_time);
}

template <typename Traits>
mapped_view_req *basic_mapped_view_kit<Traits>::make_min_time_from_now(
        const boost::chrono::system_clock::duration &min_time) {
    return new min_time_from_now_mapped_view_req(min_time);
}

template <typename Traits>
mapped_view_req *basic_mapped_view_kit<Traits>::make_max_time_from_now(
        const boost::chrono::system_clock::duration &max_time) {
    return new max_time_from_now_mapped_view_req(max_time);
}

} // memhook

#endif // MEMHOOK_BASIC_MAPPED_VIEW_KIT_HPP_INCLUDED
