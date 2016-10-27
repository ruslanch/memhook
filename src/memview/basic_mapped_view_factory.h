#ifndef MEMHOOK_SRC_MEMVIEW_BASIC_MAPPED_VIEW_FACTORY_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_BASIC_MAPPED_VIEW_FACTORY_H_INCLUDED

#include "common.h"
#include "basic_mapped_view.h"
#include "basic_simple_mapped_view.h"
#include "basic_aggregated_mapped_view.h"
#include "mapped_view_options.h"

namespace memhook
{

template <typename Traits>
struct BasicMappedViewFactory : MappedViewFactory
{
    unique_ptr<MappedView>       NewSimpleView(const char *name);
    unique_ptr<MappedView>       NewAggregatedView(const char *name);
    unique_ptr<MappedViewOption> NewMinSizeOption(std::size_t minsize);
    unique_ptr<MappedViewOption> NewMinTimeOption(const boost::chrono::system_clock::time_point &min_time);
    unique_ptr<MappedViewOption> NewMaxTimeOption(const boost::chrono::system_clock::time_point &max_time);
    unique_ptr<MappedViewOption> NewMinTimeFromNowOption(const boost::chrono::system_clock::duration &min_time);
    unique_ptr<MappedViewOption> NewMaxTimeFromNowOption(const boost::chrono::system_clock::duration &max_time);
};

template <typename Traits>
unique_ptr<MappedView> BasicMappedViewFactory<Traits>::NewSimpleView(const char *name)
{
    return unique_ptr<MappedView>(new BasicSimpleMappedView<Traits>(name));
}

template <typename Traits>
unique_ptr<MappedView> BasicMappedViewFactory<Traits>::NewAggregatedView(const char *name)
{
    return unique_ptr<MappedView>(new BasicAggregatedMappedView<Traits>(name));
}

template <typename Traits>
unique_ptr<MappedViewOption> BasicMappedViewFactory<Traits>::NewMinSizeOption(std::size_t min_size)
{
    return unique_ptr<MappedViewOption>(new MinSizeMappedViewOption(min_size));
}

template <typename Traits>
unique_ptr<MappedViewOption> BasicMappedViewFactory<Traits>::NewMinTimeOption(
        const boost::chrono::system_clock::time_point &min_time)
{
    return unique_ptr<MappedViewOption>(new MinTimeMappedViewOption(min_time));
}

template <typename Traits>
unique_ptr<MappedViewOption> BasicMappedViewFactory<Traits>::NewMaxTimeOption(
        const boost::chrono::system_clock::time_point &max_time)
{
    return unique_ptr<MappedViewOption>(new MaxTimeMappedViewOption(max_time));
}

template <typename Traits>
unique_ptr<MappedViewOption> BasicMappedViewFactory<Traits>::NewMinTimeFromNowOption(
        const boost::chrono::system_clock::duration &min_time)
{
    return unique_ptr<MappedViewOption>(new MinTimeFromNowMappedViewOption(min_time));
}

template <typename Traits>
unique_ptr<MappedViewOption> BasicMappedViewFactory<Traits>::NewMaxTimeFromNowOption(
        const boost::chrono::system_clock::duration &max_time)
{
    return unique_ptr<MappedViewOption>(new MaxTimeFromNowMappedViewOption(max_time));
}

} // memhook

#endif
