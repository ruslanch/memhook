#ifndef MEMHOOK_BASIC_SIMPLE_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_SIMPLE_MAPPED_VIEW_HPP_INCLUDED

#include "basic_mapped_view.hpp"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_stream.hpp>

namespace memhook {

template <typename Traits>
struct basic_simple_mapped_view : basic_mapped_view<Traits> {
    typedef basic_mapped_view<Traits>            base_t;
    typedef typename base_t::traceinfo_t         traceinfo_t;
    typedef typename base_t::mapped_view_req_t   mapped_view_req_t;
    typedef typename base_t::indexed_container_t indexed_container_t;

    explicit basic_simple_mapped_view(const char *name)
            : base_t(name) {
    }

protected:
    void do_write(std::ostream &os);

private:
    struct indexed_container_printer : std::unary_function<traceinfo_t, bool> {
        typedef spirit::karma::ostream_iterator<char> ostream_iterator_t;
        basic_simple_mapped_view *view_;
        std::ostream      *os_;

        indexed_container_printer(basic_simple_mapped_view *view, std::ostream &os)
                : view_(view)
                , os_(&os) {
        }

        bool operator()(const traceinfo_t &tinfo) const
        {
            if (mapped_view_detail::is_interrupted())
                return false;

            if (find_if(view_->reqs_, !bind(&mapped_view_req_t::invoke, _1, cref(tinfo)))
                    != view_->reqs_.end())
                return true;

            const time_t ttsec = system_clock_t::to_time_t(tinfo.timestamp);
            const int_least64_t nsec = (tinfo.timestamp - system_clock_t::from_time_t(ttsec)).count();
            struct tm tm = {0};
            (void)localtime_r(&ttsec, &tm);

            ostream_iterator_t sink(*os_);
            using namespace spirit::karma;
            generate(sink, "0x" << hex
                << ", size="
                << ulong_long
                << ", ts="
                << right_align(4, '0')[int_]
                << '-'
                << right_align(2, '0')[int_]
                << '-'
                << right_align(2, '0')[int_]
                << ' '
                << right_align(2, '0')[int_]
                << ':'
                << right_align(2, '0')[int_]
                << ':'
                << right_align(2, '0')[int_]
                << '.'
                << ulong_long
                << '\n'
                , tinfo.address
                , tinfo.memsize
                , tm.tm_year + 1900
                , tm.tm_mon + 1
                , tm.tm_mday
                , tm.tm_hour
                , tm.tm_min
                , tm.tm_sec
                , nsec
            );

            if (view_->is_show_callstack())
                for_each(tinfo.callstack, *this);

            return true;
        }

        void operator()(const traceinfo_callstack_item &item) const {
            *os_ << "  [ip=0x" << std::hex << item.ip << ", sp=0x" << item.sp << "] "
                 << view_->resolve_shl_path(item.shl_addr)
                 << '(' << view_->cxa_demangle(view_->resolve_procname(item.ip)).get()
                 << " +0x" << item.offp << ")\n";
        }
    };
};

template <typename Traits>
void basic_simple_mapped_view<Traits>::do_write(std::ostream &os) {
    if (this->is_sort_by_address())
    {
        typedef typename indexed_container_t::template nth_index<0>::type index0;
        index0 &idx = get<0>(this->container->indexed_container);
        indexed_container_printer printer(this, os);
        std::find_if(idx.begin(), idx.end(), std::not1(indexed_container_printer(this, os)));
    }
    else if (this->is_sort_by_size())
    {
        typedef typename indexed_container_t::template nth_index<2>::type index2;
        index2 &idx = get<2>(this->container->indexed_container);
        std::find_if(idx.begin(), idx.end(), std::not1(indexed_container_printer(this, os)));
    }
    else
    {
        typedef typename indexed_container_t::template nth_index<1>::type index1;
        index1 &idx = get<1>(this->container->indexed_container);
        std::find_if(idx.begin(), idx.end(), std::not1(indexed_container_printer(this, os)));
    }
}

} // memhook

#endif // MEMHOOK_BASIC_SIMPLE_MAPPED_VIEW_HPP_INCLUDED
