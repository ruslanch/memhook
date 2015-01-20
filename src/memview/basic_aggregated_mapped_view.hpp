#ifndef MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
#define MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED

#include "basic_mapped_view.hpp"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_stream.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/range/numeric.hpp>

namespace memhook {

template <typename Traits>
struct basic_aggregated_mapped_view : basic_mapped_view<Traits> {
    typedef basic_mapped_view<Traits>            base_t;
    typedef typename base_t::traceinfo_t         traceinfo_t;
    typedef typename base_t::indexed_container_t indexed_container_t;

    explicit basic_aggregated_mapped_view(const char *name) : base_t(name) {}

protected:
    void do_write(std::ostream &os);

private:
    struct callstack_item {
        uintptr_t shl_addr;
        uintptr_t ip;

        callstack_item(uintptr_t shl_addr, uintptr_t ip)
            : shl_addr(shl_addr), ip(ip) {}
    };

    struct callstack_item_builder : std::unary_function<traceinfo_callstack_item, callstack_item> {
        callstack_item operator()(const traceinfo_callstack_item &item) const {
            return callstack_item(item.shl_addr, item.ip);
        }
    };

    typedef std::vector<callstack_item> callstack_container;

    struct traceinfo {
        std::size_t callstack_hash;
        std::size_t memsize;
        std::size_t times;
        callstack_container callstack;

        template <typename Callstack>
        traceinfo(std::size_t callstack_hash, std::size_t memsize, std::size_t times,
                    const Callstack &a_callstack)
                : callstack_hash(callstack_hash)
                , memsize(memsize)
                , times(times) {
            callstack.reserve(a_callstack.size());
            transform(a_callstack, std::back_inserter(callstack), callstack_item_builder());
        }
    };

    typedef multi_index_container<
        traceinfo,
        multi_index::indexed_by<
            multi_index::hashed_unique<
                multi_index::member<traceinfo, std::size_t, &traceinfo::callstack_hash>
            >,
            multi_index::ordered_non_unique<
                multi_index::member<traceinfo, std::size_t, &traceinfo::memsize>
            >,
            multi_index::ordered_non_unique<
                multi_index::member<traceinfo, std::size_t, &traceinfo::times>
            >
        >
    > indexed_container;

    struct indexed_container_builder : std::unary_function<memhook::traceinfo<Traits>, bool> {
        base_t            *view_;
        indexed_container *container_;

        struct callstack_hasher : std::binary_function<std::size_t, traceinfo_callstack_item, std::size_t> {
            std::size_t operator()(std::size_t hash, const traceinfo_callstack_item &callstack_item) const {
                hash_combine(hash, callstack_item.ip);
                return hash;
            }
        };

        struct traceinfo_fields_updater {
            std::size_t memsize;
            explicit traceinfo_fields_updater(std::size_t memsize) : memsize(memsize) {}
            void operator()(traceinfo &tinfo) const {
                tinfo.memsize += memsize;
                tinfo.times   += 1;
            }
        };

        explicit indexed_container_builder(base_t *view, indexed_container &container)
            : view_(view)
            , container_(&container) {}

        bool operator()(const traceinfo_t &tinfo) const {
            if (mapped_view_detail::is_interrupted())
                return false;

            if (find_if(view_->reqs_, !bind(&basic_mapped_view_req<Traits>::invoke, _1, cref(tinfo)))
                    != view_->reqs_.end())
                return true;

            const std::size_t callstack_hash = accumulate(tinfo.callstack, 0, callstack_hasher());
            typedef typename indexed_container::template nth_index<0>::type index0;
            index0 &idx = get<0>(*container_);
            typename index0::iterator iter = idx.find(callstack_hash);
            if (iter != idx.end()) {
                idx.modify(iter, traceinfo_fields_updater(tinfo.memsize));
            } else {
                container_->emplace(callstack_hash, tinfo.memsize, 1, tinfo.callstack);
            }
            return true;
        }
    };

    struct indexed_container_printer : std::unary_function<traceinfo, bool> {
        base_t       *view_;
        std::ostream *os_;

        indexed_container_printer(base_t *view, std::ostream &os)
                : view_(view), os_(&os) {}

        bool operator()(const traceinfo &tinfo) const
        {
            if (mapped_view_detail::is_interrupted())
                return false;

            spirit::karma::ostream_iterator<char> sink(*os_);
            using namespace spirit::karma;
            generate(sink,
                "size="
                << ulong_long
                << ", times="
                << ulong_long
                << "\n",
                tinfo.memsize,
                tinfo.times);

            if (view_->is_show_callstack())
                for_each(tinfo.callstack, *this);

            return true;
        }

        void operator()(const callstack_item &item) const {
            *os_ << "  [ip=0x" << std::hex << item.ip << "] "
                 << view_->resolve_shl_path(item.shl_addr)
                 << '(' << view_->cxa_demangle(view_->resolve_procname(item.ip)).get()
                 << ")\n";
        }
    };
};

template <typename Traits>
void basic_aggregated_mapped_view<Traits>::do_write(std::ostream &os) {
    indexed_container indexed_container;
    indexed_container_builder builder(this, indexed_container);

    typedef typename indexed_container_t::template nth_index<0>::type index0;
    index0 &idx = get<0>(this->container->indexed_container);
    if (std::find_if(idx.begin(), idx.end(), std::not1(builder)) != idx.end())
        return;

    indexed_container_printer printer(this, os);
    if (this->is_sort_by_size())
    {
        typedef typename indexed_container::template nth_index<1>::type index1;
        index1 &idx = get<1>(indexed_container);
        std::find_if(idx.begin(), idx.end(), std::not1(printer));
    }
    else
    {
        typedef typename indexed_container::template nth_index<2>::type index2;
        index2 &idx = get<2>(indexed_container);
        std::find_if(idx.begin(), idx.end(), std::not1(printer));
    }
}

} // memhook

#endif // MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_HPP_INCLUDED
