#ifndef MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED

#include "basic_mapped_view.hpp"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_stream.hpp>
#include <boost/container/vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/range/numeric.hpp>

namespace memhook {

struct aggregated_traceinfo {
    typedef container::vector<traceinfo_callstack_item>
            callstack_container;

    std::size_t         hash;
    std::size_t         memsize;
    std::size_t         times;
    callstack_container callstack;

    template <typename T, typename A>
    aggregated_traceinfo(std::size_t hash, std::size_t memsize, std::size_t times,
                const container::vector<T, A> &a_callstack)
            : hash(hash)
            , memsize(memsize)
            , times(times)
            , callstack(a_callstack.begin(), a_callstack.end()) {}
};

typedef multi_index_container<
    aggregated_traceinfo,
    multi_index::indexed_by<
        multi_index::hashed_unique<
            multi_index::member<aggregated_traceinfo, std::size_t, &aggregated_traceinfo::hash>
        >,
        multi_index::ordered_non_unique<
            multi_index::member<aggregated_traceinfo, std::size_t, &aggregated_traceinfo::memsize>
        >,
        multi_index::ordered_non_unique<
            multi_index::member<aggregated_traceinfo, std::size_t, &aggregated_traceinfo::times>
        >
    >
> aggregated_indexed_container;

template <typename Traits>
struct aggregated_indexed_container_builder : std::unary_function<traceinfo<Traits>, bool> {
    typedef basic_mapped_view<Traits>     mapped_view_t;
    typedef traceinfo<Traits>             traceinfo_t;
    typedef aggregated_indexed_container  indexed_container_t;

    mapped_view_t       &view;
    indexed_container_t &indexed_container;

    aggregated_indexed_container_builder(mapped_view_t &view, indexed_container_t &indexed_container)
            : view(view)
            , indexed_container(indexed_container) {}

    struct traceinfo_callstack_item_by_ip_hasher {
        std::size_t operator()(std::size_t hash, const traceinfo_callstack_item &item) const {
            hash_combine(hash, item.ip);
            hash_combine(hash, item.sp);
            return hash;
        }
    };

    struct aggregated_traceinfo_fields_updater {
        std::size_t memsize;
        explicit aggregated_traceinfo_fields_updater(std::size_t memsize) : memsize(memsize) {}
        void operator()(aggregated_traceinfo &tinfo) const {
            tinfo.memsize += memsize;
            tinfo.times   += 1;
        }
    };

    bool operator()(const traceinfo_t &tinfo) const {
        if (mapped_view_detail::is_interrupted())
            return false;

        if (view.check_requirements(tinfo)) {
            const std::size_t hash = accumulate(tinfo.callstack, 0, traceinfo_callstack_item_by_ip_hasher());
            typedef indexed_container_t::nth_index<0>::type index0;
            index0 &idx = get<0>(indexed_container);
            index0::iterator iter = idx.find(hash);
            if (iter != idx.end()) {
                idx.modify(iter, aggregated_traceinfo_fields_updater(tinfo.memsize));
            } else {
                indexed_container.emplace(hash, tinfo.memsize, 1, tinfo.callstack);
            }
        }
        return true;
    }
};

template <typename Traits>
struct aggregated_indexed_container_printer : std::unary_function<aggregated_traceinfo, bool> {
    typedef basic_mapped_view<Traits> mapped_view_t;

    mapped_view_t &view;
    std::ostream  &os;

    aggregated_indexed_container_printer(mapped_view_t &view, std::ostream &os)
            : view(view), os(os) {}

    bool operator()(const aggregated_traceinfo &tinfo) const {
        if (mapped_view_detail::is_interrupted())
            return false;

        using namespace spirit::karma;
        spirit::karma::ostream_iterator<char> sink(os);
        generate(sink, "size=" << ulong_long << ", times=" << ulong_long << "\n",
                tinfo.memsize, tinfo.times);

        if (view.is_show_callstack())
            for_each(tinfo.callstack, *this);

        return true;
    }

    void operator()(const traceinfo_callstack_item &item) const {
        os << "  [ip=0x" << std::hex << item.ip << ", sp=0x" << item.sp << "] "
           << view.resolve_shl_path(item.shl_addr) << '('
           << view.cxa_demangle(view.resolve_procname(item.ip)).get()
           << " +0x" << item.offp << ")\n";
    }
};

template <typename Traits>
struct basic_aggregated_mapped_view : basic_mapped_view<Traits> {
    typedef basic_mapped_view<Traits>                   mapped_view_t;
    typedef typename mapped_view_t::indexed_container_t indexed_container_t;
    explicit basic_aggregated_mapped_view(const char *name) : mapped_view_t(name) {}
protected:
    void do_write(std::ostream &os);
};

template <typename Traits>
void basic_aggregated_mapped_view<Traits>::do_write(std::ostream &os) {
    aggregated_indexed_container indexed_container;
    aggregated_indexed_container_builder<Traits> builder(*this, indexed_container);

    typedef typename indexed_container_t::template nth_index<0>::type index0;
    index0 &idx = get<0>(this->container->indexed_container);
    if (std::find_if(idx.begin(), idx.end(), std::not1(builder)) != idx.end())
        return;

    aggregated_indexed_container_printer<Traits> printer(*this, os);
    if (this->is_sort_by_size())
    {
        typedef aggregated_indexed_container::nth_index<1>::type index1;
        index1 &idx = get<1>(indexed_container);
        std::find_if(idx.begin(), idx.end(), std::not1(printer));
    }
    else
    {
        typedef aggregated_indexed_container::nth_index<2>::type index2;
        index2 &idx = get<2>(indexed_container);
        std::find_if(idx.begin(), idx.end(), std::not1(printer));
    }
}

} // memhook

#endif // MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
