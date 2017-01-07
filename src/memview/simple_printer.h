#ifndef MEMHOOK_SRC_MEMVIEW_SIMPLE_PRINTER_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_SIMPLE_PRINTER_H_INCLUDED

#include "common.h"
#include "mapped_view_base.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_stream.hpp>

namespace memhook {
  struct SimpleTraceInfoPrinter {
    const MappedViewBase &view;
    std::ostream &os;

    SimpleTraceInfoPrinter(const MappedViewBase &view, std::ostream &os)
        : view(view)
        , os(os) {}

    template <typename TraceInfoT>
    bool operator()(const TraceInfoT &tinfo) const {
      if (view.IsInterrupted())
        return false;

      if (view.CheckTraceInfoOptions(tinfo)) {
        const time_t ttsec = chrono::system_clock::to_time_t(tinfo.timestamp);
        const int_least64_t nsec = (tinfo.timestamp - chrono::system_clock::from_time_t(ttsec)).count();

        struct tm tm = {0};
        (void)localtime_r(&ttsec, &tm);

        using namespace boost::spirit::karma;
        ostream_iterator<char> sink(os);
        generate(sink,
                "0x" << hex << ", size=" << ulong_long << ", ts=" << right_align(4, '0')[int_]
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
                     << '\n',
                tinfo.address,
                tinfo.memsize,
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                nsec);

        if (view.GetOptionFlag(MappedView::kShowCallStack))
          boost::for_each(tinfo.callstack, *this);
      }
      return true;
    }

    void operator()(const TraceInfoCallStackItem &item) const {
      os << "  [ip=0x" << std::hex << item.ip << ", sp=0x" << item.sp << "] "
         << view.GetModulePath(item.shl_addr).get() << '('
         << CxxSymbolDemangle(view.GetProcName(item.ip).get()).get() << " +0x"
         << item.offp << ")\n";
    }
  };
}

#endif
