#ifndef MEMHOOK_BASIC_MAPPED_VIEW_KIT_HPP_INCLUDED
#define MEMHOOK_BASIC_MAPPED_VIEW_KIT_HPP_INCLUDED

#include "basic_mapped_view.hpp"
#include "basic_simple_mapped_view.hpp"
#include "basic_aggregated_mapped_view.hpp"
#include "basic_mapped_view_req.hpp"


namespace memhook {

template <typename Traits>
struct basic_mapped_view_kit : mapped_view_kit {
    basic_mapped_view<Traits>     *make_view(const char *name);
    basic_mapped_view<Traits>     *make_aggregated_view(const char *name);
    basic_mapped_view_req<Traits> *make_min_size_req(std::size_t minsize);
    basic_mapped_view_req<Traits> *make_min_time_req(const system_clock_t::time_point &min_time);
    basic_mapped_view_req<Traits> *make_max_time_req(const system_clock_t::time_point &max_time);
    basic_mapped_view_req<Traits> *make_min_time_from_now_req(const system_clock_t::duration &min_time);
    basic_mapped_view_req<Traits> *make_max_time_from_now_req(const system_clock_t::duration &max_time);
};

template <typename Traits>
basic_mapped_view<Traits> *basic_mapped_view_kit<Traits>::make_view(const char *name) {
    return new basic_simple_mapped_view<Traits>(name);
}

template <typename Traits>
basic_mapped_view<Traits> *basic_mapped_view_kit<Traits>::make_aggregated_view(const char *name) {
    return new basic_aggregated_mapped_view<Traits>(name);
}

template <typename Traits>
basic_mapped_view_req<Traits> *basic_mapped_view_kit<Traits>::make_min_size_req(std::size_t minsize) {
    return new basic_min_size_mapped_view_req<Traits>(minsize);
}

template <typename Traits>
basic_mapped_view_req<Traits> *basic_mapped_view_kit<Traits>::make_min_time_req(
        const system_clock_t::time_point &min_time) {
    return new basic_min_time_mapped_view_req<Traits>(min_time);
}

template <typename Traits>
basic_mapped_view_req<Traits> *basic_mapped_view_kit<Traits>::make_max_time_req(
        const system_clock_t::time_point &max_time) {
    return new basic_max_time_mapped_view_req<Traits>(max_time);
}

template <typename Traits>
basic_mapped_view_req<Traits> *basic_mapped_view_kit<Traits>::make_min_time_from_now_req(
        const system_clock_t::duration &min_time) {
    return new basic_min_time_from_now_mapped_view_req<Traits>(min_time);
}

template <typename Traits>
basic_mapped_view_req<Traits> *basic_mapped_view_kit<Traits>::make_max_time_from_now_req(
        const system_clock_t::duration &max_time) {
    return new basic_max_time_from_now_mapped_view_req<Traits>(max_time);
}

} // memhook

#endif // MEMHOOK_BASIC_MAPPED_VIEW_KIT_HPP_INCLUDED
