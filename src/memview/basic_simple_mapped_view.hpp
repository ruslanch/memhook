#ifndef MEMHOOK_BASIC_SIMPLE_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_SIMPLE_MAPPED_VIEW_HPP_INCLUDED

#include <memhook/common.hpp>
#include "basic_mapped_view.hpp"
#include "simple_printer.hpp"
#include <boost/range/algorithm/find_if.hpp>
#include <boost/bind.hpp>

namespace memhook {

template <typename Traits>
struct basic_simple_mapped_view : basic_mapped_view<Traits> {
    explicit basic_simple_mapped_view(const char *name)
            : basic_mapped_view<Traits>(name) {}
protected:
    void do_write(std::ostream &os) {
        if (this->sort_by_address())
            for_each_do(get<0>(this->get_indexed_container()), os);
        else if (this->sort_by_size())
            for_each_do(get<2>(this->get_indexed_container()), os);
        else
            for_each_do(get<1>(this->get_indexed_container()), os);
    }

    template <typename Index>
    void for_each_do(const Index &index, std::ostream &os) const {
        simple_traceinfo_printer printer(*this, os);
        range::find_if(index, !bind<bool>(printer, _1));
    }
};

} // memhook

#endif // MEMHOOK_BASIC_SIMPLE_MAPPED_VIEW_HPP_INCLUDED
