#ifndef MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_H_INCLUDED
#define MEMHOOK_BASIC_AGGREGATED_MAPPED_VIEW_H_INCLUDED

#include "common.h"
#include "aggregated_printer.h"
#include "basic_mapped_view.h"

#include <boost/range/algorithm/find_if.hpp>

namespace memhook {
  template <typename Traits>
  struct BasicAggregatedMappedView : BasicMappedView<Traits> {
    explicit BasicAggregatedMappedView(const char *name)
        : BasicMappedView<Traits>(name) {}

  protected:
    void WriteImpl(std::ostream &os) {
      ForEachDo(boost::get<1>(this->GetIndexedContainer()), os);
    }

    template <typename Index>
    void ForEachDo(const Index &index, std::ostream &os) const {
      AggregatedIndexedContainer indexed_container;
      AggregatedIndexedContainerBuilder builder(*this, indexed_container);
      if (boost::range::find_if(index, !boost::bind<bool>(builder, _1)) != index.end())
        return;

      if (this->GetOptionFlag(MappedView::kSortBySize))
        ForEachDo_2(boost::get<1>(indexed_container), os);
      else
        ForEachDo_2(boost::get<2>(indexed_container), os);
    }

    template <typename Index>
    void ForEachDo_2(const Index &index, std::ostream &os) const {
      AggregatedTraceInfoPrinter printer(*this, os);
      boost::range::find_if(index, !boost::bind<bool>(printer, _1));
    }
  };
}

#endif
