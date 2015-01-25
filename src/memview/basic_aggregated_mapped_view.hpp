#ifndef MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED

#include <memhook/common.hpp>
#include "basic_mapped_view.hpp"
#include "aggregated_printer.hpp"
#include <boost/range/algorithm/find_if.hpp>

namespace memhook {

template <typename Traits>
struct basic_aggregated_mapped_view : basic_mapped_view<Traits> {
    explicit basic_aggregated_mapped_view(const char *name)
            : basic_mapped_view<Traits>(name) {}
protected:
    void do_write(std::ostream &os) {
        for_each_do(get<0>(this->get_indexed_container()), os);
    }

    template <typename Index>
    void for_each_do(const Index &index, std::ostream &os) const {
        aggregated_indexed_container indexed_container;
        aggregated_indexed_container_builder builder(*this, indexed_container);
        if (range::find_if(index, !bind<bool>(builder, _1)) != index.end())
            return;

        if (this->sort_by_size())
            for_each_do_2(get<1>(indexed_container), os);
        else
            for_each_do_2(get<2>(indexed_container), os);
    }

    template <typename Index>
    void for_each_do_2(const Index &index, std::ostream &os) const {
        aggregated_traceinfo_printer printer(*this, os);
        range::find_if(index, !bind<bool>(printer, _1));
    }
};

} // memhook

#endif // MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
