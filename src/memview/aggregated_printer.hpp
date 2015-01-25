#ifndef MEMHOOK_AGGREGATED_PRINTER_HPP_INCLUDED
#define MEMHOOK_AGGREGATED_PRINTER_HPP_INCLUDED

#include <memhook/common.hpp>
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

struct aggregated_indexed_container_builder {
    typedef aggregated_indexed_container indexed_container_t;

    const mapped_view_base       &view;
    aggregated_indexed_container &indexed_container;

    aggregated_indexed_container_builder(const mapped_view_base &view,
            aggregated_indexed_container &indexed_container)
        : view(view)
        , indexed_container(indexed_container) {}

    struct aggregated_traceinfo_fields_updater {
        std::size_t memsize;
        explicit aggregated_traceinfo_fields_updater(std::size_t memsize) : memsize(memsize) {}
        void operator()(aggregated_traceinfo &tinfo) const {
            tinfo.memsize += memsize;
            tinfo.times   += 1;
        }
    };

    template <typename Traceinfo>
    bool operator()(const Traceinfo &tinfo) const {
        if (view.is_interrupted())
            return false;

        if (view.check_traceinfo_reqs(tinfo)) {
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

struct aggregated_traceinfo_printer {
    const mapped_view_base &view;
    std::ostream &os;

    aggregated_traceinfo_printer(const mapped_view_base &view, std::ostream &os)
            : view(view), os(os) {}

    bool operator()(const aggregated_traceinfo &tinfo) const {
        if (view.is_interrupted())
            return false;

        using namespace spirit::karma;
        spirit::karma::ostream_iterator<char> sink(os);
        generate(sink, "size=" << ulong_long << ", times=" << ulong_long << "\n",
                tinfo.memsize, tinfo.times);

        if (view.show_callstack())
            for_each(tinfo.callstack, *this);

        return true;
    }

    void operator()(const traceinfo_callstack_item &item) const {
        os << "  [ip=0x" << std::hex << item.ip << ", sp=0x" << item.sp << "] "
           << view.get_module_path(item.shl_addr).get() << '('
           << cxx_symbol_demangle(view.get_proc_name(item.ip).get()).get()
           << " +0x" << item.offp << ")\n";
    }
};

} // memhook

#endif // MEMHOOK_AGGREGATED_PRINTER_HPP_INCLUDED
