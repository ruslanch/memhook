#ifndef MEMHOOK_SRC_MEMVIEW_AGGREGATED_PRINTER_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_AGGREGATED_PRINTER_H_INCLUDED

#include "common.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_stream.hpp>
#include <boost/container/list.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/range/numeric.hpp>

namespace memhook {

typedef boost::container::vector<TraceInfoCallStackItem> AggregatedCallstack;

struct AggregatedTraceInfo
{
    std::size_t memsize;
    std::size_t times;
    AggregatedCallstack callstack;

    template <typename A>
    AggregatedTraceInfo(std::size_t memsize, std::size_t times,
            const boost::container::vector<TraceInfoCallStackItem, A> &a_callstack)
        : memsize(memsize)
        , times(times)
        , callstack(a_callstack.begin(), a_callstack.end())
    {}
};

struct TraceInfoCallstackItemHashByIpAndSp
{
    std::size_t operator()(std::size_t seed, const TraceInfoCallStackItem &item) const
    {
        boost::hash_combine(seed, item.ip);
        boost::hash_combine(seed, item.sp);
        return seed;
    }
};

struct CallstackContainerHashByIpAndSp
{
    template <typename A>
    std::size_t operator()(const boost::container::vector<TraceInfoCallStackItem, A> &callstack) const
    {
        std::size_t seed = 0;
        return boost::accumulate(callstack, seed, TraceInfoCallstackItemHashByIpAndSp());
    }
};

struct TraceInfoCallstackItemEqualByIpAndSp
{
    bool operator()(const TraceInfoCallStackItem &lhs, const TraceInfoCallStackItem &rhs) const
    {
        return lhs.ip == rhs.ip && lhs.sp == rhs.sp;
    }
};

struct CallstackContainerEqualByIpAndSp
{
    template <typename A1, typename A2>
    bool operator()(const boost::container::vector<TraceInfoCallStackItem, A1> &lhs,
                    const boost::container::vector<TraceInfoCallStackItem, A2> &rhs) const
    {
        if (lhs.size() != rhs.size())
            return false;
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(),
            TraceInfoCallstackItemEqualByIpAndSp());
    }
};

typedef boost::multi_index_container<
    AggregatedTraceInfo,
    boost::multi_index::indexed_by<
        boost::multi_index::hashed_unique<
            boost::multi_index::member<AggregatedTraceInfo, AggregatedCallstack, &AggregatedTraceInfo::callstack>,
            CallstackContainerHashByIpAndSp, CallstackContainerEqualByIpAndSp
        >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::member<AggregatedTraceInfo, std::size_t, &AggregatedTraceInfo::memsize>
        >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::member<AggregatedTraceInfo, std::size_t, &AggregatedTraceInfo::times>
        >
    >
> AggregatedIndexedContainer;

struct AggregatedIndexedContainerBuilder
{
    typedef AggregatedIndexedContainer IndexedContainer;

    const MappedViewBase       &view;
    AggregatedIndexedContainer &indexed_container;

    AggregatedIndexedContainerBuilder(const MappedViewBase &view,
            AggregatedIndexedContainer &indexed_container)
        : view(view)
        , indexed_container(indexed_container)
    {}

    struct AggregatedTraceInfoFieldsUpdater
    {
        std::size_t memsize;

        explicit AggregatedTraceInfoFieldsUpdater(std::size_t memsize)
            : memsize(memsize)
        {}

        void operator()(AggregatedTraceInfo &tinfo) const
        {
            tinfo.memsize += memsize;
            tinfo.times   += 1;
        }
    };

    template <typename TraceInfoT>
    bool operator()(const TraceInfoT &tinfo) const
    {
        if (view.IsInterrupted())
            return false;

        if (view.CheckTraceInfoOptions(tinfo))
        {
            typedef IndexedContainer::nth_index<0>::type index0;
            index0 &idx = boost::get<0>(indexed_container);
            index0::iterator iter = idx.find(tinfo.callstack);
            if (iter != idx.end())
            {
                idx.modify(iter, AggregatedTraceInfoFieldsUpdater(tinfo.memsize));
            }
            else
            {
                indexed_container.emplace(tinfo.memsize, 1, tinfo.callstack);
            }
        }
        return true;
    }
};

struct AggregatedTraceInfoPrinter
{
    const MappedViewBase &view;
    std::ostream &os;

    AggregatedTraceInfoPrinter(const MappedViewBase &view, std::ostream &os)
        : view(view), os(os)
    {}

    bool operator()(const AggregatedTraceInfo &tinfo) const
    {
        if (view.IsInterrupted())
            return false;

        using namespace boost::spirit::karma;
        ostream_iterator<char> sink(os);
        generate(sink, "size=" << ulong_long << ", times=" << ulong_long << "\n",
                tinfo.memsize, tinfo.times);

        if (view.GetOptionFlag(MappedView::ShowCallStack))
            boost::for_each(tinfo.callstack, *this);

        return true;
    }

    void operator()(const TraceInfoCallStackItem &item) const
    {
        os << "  [ip=0x" << std::hex << item.ip << ", sp=0x" << item.sp << "] "
           << view.GetModulePath(item.shl_addr).get() << '('
           << CxxSymbolDemangle(view.GetProcName(item.ip).get()).get()
           << " +0x" << item.offp << ")\n";
    }
};

} // memhook

#endif
