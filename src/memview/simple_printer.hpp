#ifndef MEMHOOK_SIMPLE_PRINTER_HPP_INCLUDED
#define MEMHOOK_SIMPLE_PRINTER_HPP_INCLUDED

#include <memhook/common.hpp>
#include "mapped_view_base.hpp"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_stream.hpp>

namespace memhook {

struct simple_traceinfo_printer {
    const mapped_view_base &view;
    std::ostream &os;

    simple_traceinfo_printer(const mapped_view_base &view, std::ostream &os)
            : view(view), os(os) {}

    template <typename Traceinfo>
    bool operator()(const Traceinfo &tinfo) const {
        if (view.is_interrupted())
            return false;

        if (view.check_traceinfo_reqs(tinfo)) {
            const time_t ttsec = system_clock_t::to_time_t(tinfo.timestamp());
            const int_least64_t nsec = (tinfo.timestamp() - system_clock_t::from_time_t(ttsec)).count();
            struct tm tm = {0};
            (void)localtime_r(&ttsec, &tm);

            using namespace spirit::karma;
            spirit::karma::ostream_iterator<char> sink(os);
            generate(sink, "0x" << hex << ", size=" << ulong_long
                << ", ts="
                << right_align(4, '0')[int_] << '-' << right_align(2, '0')[int_] << '-' << right_align(2, '0')[int_]
                << ' '
                << right_align(2, '0')[int_] << ':' << right_align(2, '0')[int_] << ':' << right_align(2, '0')[int_]
                << '.' << ulong_long << '\n'
                , tinfo.address(), tinfo.memsize()
                , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday
                , tm.tm_hour, tm.tm_min, tm.tm_sec
                , nsec
            );

            if (view.show_callstack())
                for_each(tinfo.callstack(), *this);
        }
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

#endif // MEMHOOK_SIMPLE_PRINTER_HPP_INCLUDED
