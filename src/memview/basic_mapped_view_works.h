#ifndef MEMHOOK_SRC_MEMVIEW_BASIC_MAPPED_VIEW_FACTORY_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_BASIC_MAPPED_VIEW_FACTORY_H_INCLUDED

#include "common.h"
#include "basic_aggregated_mapped_view.h"
#include "basic_mapped_view.h"
#include "basic_simple_mapped_view.h"
#include "mapped_view_options.h"

namespace memhook {
  template <typename Traits>
  struct BasicMappedViewWorks : MappedViewWorks {
    unique_ptr<MappedView> NewSimpleView(const char *name) {
      return unique_ptr<MappedView>(new BasicSimpleMappedView<Traits>(name));
    }

    unique_ptr<MappedView> NewAggregatedView(const char *name) {
      return unique_ptr<MappedView>(new BasicAggregatedMappedView<Traits>(name));
    }

    unique_ptr<MappedViewOption> NewMinSizeOption(std::size_t min_size) {
      return unique_ptr<MappedViewOption>(new MinSizeMappedViewOption(min_size));
    }

    unique_ptr<MappedViewOption> NewMinTimeOption(const chrono::system_clock::time_point &min_time) {
      return unique_ptr<MappedViewOption>(new MinTimeMappedViewOption(min_time));
    }

    unique_ptr<MappedViewOption> NewMaxTimeOption(const chrono::system_clock::time_point &max_time) {
      return unique_ptr<MappedViewOption>(new MaxTimeMappedViewOption(max_time));
    }

    unique_ptr<MappedViewOption> NewMinTimeFromNowOption(const chrono::system_clock::duration &min_time) {
      return unique_ptr<MappedViewOption>(new MinTimeFromNowMappedViewOption(min_time));
    }

    unique_ptr<MappedViewOption> NewMaxTimeFromNowOption(const chrono::system_clock::duration &max_time) {
      return unique_ptr<MappedViewOption>(new MaxTimeFromNowMappedViewOption(max_time));
    }

    unique_ptr<MappedViewOption> NewMinTimeFromStartOption(const chrono::system_clock::duration &min_time) {
      return unique_ptr<MappedViewOption>(new MinTimeFromStartMappedViewOption(min_time));
    }

    unique_ptr<MappedViewOption> NewMaxTimeFromStartOption(const chrono::system_clock::duration &max_time) {
      return unique_ptr<MappedViewOption>(new MaxTimeFromStartMappedViewOption(max_time));
    }
  };
}

#endif
