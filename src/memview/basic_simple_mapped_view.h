#ifndef MEMHOOK_SRC_MEMVIEW_BASIC_SIMPLE_MAPPED_VIEW_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_BASIC_SIMPLE_MAPPED_VIEW_H_INCLUDED

#include "common.h"
#include "basic_mapped_view.h"
#include "simple_printer.h"

#include <boost/bind.hpp>
#include <boost/range/algorithm/find_if.hpp>

namespace memhook {
  template <typename Traits>
  struct BasicSimpleMappedView : BasicMappedView<Traits> {
    explicit BasicSimpleMappedView(const char *name)
        : BasicMappedView<Traits>(name) {}

  protected:
    void WriteImpl(std::ostream &os) {
      if (this->GetOptionFlag(MappedView::kSortByAddress))
        ForEachDo(boost::get<0>(this->GetIndexedContainer()), os);
      else if (this->GetOptionFlag(MappedView::kSortBySize))
        ForEachDo(boost::get<2>(this->GetIndexedContainer()), os);
      else
        ForEachDo(boost::get<1>(this->GetIndexedContainer()), os);
    }

    template <typename Index>
    void ForEachDo(const Index &index, std::ostream &os) const {
      SimpleTraceInfoPrinter printer(*this, os);
      boost::range::find_if(index, !boost::bind<bool>(printer, _1));
    }
  };
}

#endif
