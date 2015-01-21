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
#include <boost/range/algorithm/find_if.hpp>

namespace memhook {

typedef container::vector<traceinfo_callstack_item>
    aggregated_callstack;

struct aggregated_traceinfo {
    std::size_t memsize;
    std::size_t times;
    aggregated_callstack callstack;

    template <typename A>
    aggregated_traceinfo(std::size_t memsize, std::size_t times,
                const container::vector<traceinfo_callstack_item, A> &a_callstack)
            : memsize(memsize)
            , times(times)
            , callstack(a_callstack.begin(), a_callstack.end()) {}
};

struct traceinfo_callstack_item_hash_by_ip_and_sp {
    std::size_t operator()(std::size_t seed, const traceinfo_callstack_item &item) const {
        hash_combine(seed, item.ip);
        hash_combine(seed, item.sp);
        return seed;
    }
};

struct callstack_container_hash_by_ip_and_sp {
    template <typename A>
    std::size_t operator()(const container::vector<traceinfo_callstack_item, A> &callstack) const {
        std::size_t seed = 0;
        return accumulate(callstack, seed, traceinfo_callstack_item_hash_by_ip_and_sp());
    }
};

struct traceinfo_callstack_item_equal_by_ip_and_sp {
    bool operator()(const traceinfo_callstack_item &lhs, const traceinfo_callstack_item &rhs) const {
        return lhs.ip == rhs.ip && lhs.sp == rhs.sp;
    }
};

struct callstack_container_equal_by_ip_and_sp {
    template <typename A1, typename A2>
    bool operator()(const container::vector<traceinfo_callstack_item, A1> &lhs,
                    const container::vector<traceinfo_callstack_item, A2> &rhs) const {
        if (lhs.size() != rhs.size())
            return false;
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(),
            traceinfo_callstack_item_equal_by_ip_and_sp());
    }
};

typedef multi_index_container<
    aggregated_traceinfo,
    multi_index::indexed_by<
        multi_index::hashed_unique<
            multi_index::member<aggregated_traceinfo, aggregated_callstack, &aggregated_traceinfo::callstack>,
            callstack_container_hash_by_ip_and_sp, callstack_container_equal_by_ip_and_sp
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
            typedef indexed_container_t::nth_index<0>::type index0;
            index0 &idx = get<0>(indexed_container);
            index0::iterator iter = idx.find(tinfo.callstack);
            if (iter != idx.end()) {
                idx.modify(iter, aggregated_traceinfo_fields_updater(tinfo.memsize));
            } else {
                indexed_container.emplace(tinfo.memsize, 1, tinfo.callstack);
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
    if (range::find_if(idx, std::not1(builder)) != idx.end())
        return;

    aggregated_indexed_container_printer<Traits> printer(*this, os);
    if (this->is_sort_by_size())
    {
        typedef aggregated_indexed_container::nth_index<1>::type index1;
        index1 &idx = get<1>(indexed_container);
        range::find_if(idx, std::not1(printer));
    }
    else
    {
        typedef aggregated_indexed_container::nth_index<2>::type index2;
        index2 &idx = get<2>(indexed_container);
        range::find_if(idx, std::not1(printer));
    }
}

} // memhook

#endif // MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
