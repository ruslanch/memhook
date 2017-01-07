#ifndef MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_H_INCLUDED

#include "common.h"
#include "mapped_view_options.h"

#include <boost/chrono/system_clocks.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <iosfwd>

namespace memhook {
  class MappedView : noncopyable {
  public:
    enum OptionFlag {
      kNoLock        = 0x0001,
      kShowCallStack = 0x0002,
      kSortByAddress = 0x0010,
      kSortByTime    = 0x0020,
      kSortBySize    = 0x0040,
    };

    virtual ~MappedView() {}
    virtual void Write(std::ostream &os) = 0;
    virtual void SetOption(BOOST_RV_REF(unique_ptr<MappedViewOption>) option) = 0;
    virtual void SetOptionFlag(OptionFlag option_flag, bool setval) = 0;
    virtual bool GetOptionFlag(OptionFlag option_flag) const = 0;
    virtual std::size_t GetSize() = 0;
    virtual std::size_t GetFreeMemory() = 0;
  };

  class MappedViewWorks : noncopyable {
  public:
    virtual ~MappedViewWorks() {}
    virtual unique_ptr<MappedView> NewSimpleView(const char *name) = 0;
    virtual unique_ptr<MappedView> NewAggregatedView(const char *name) = 0;
    virtual unique_ptr<MappedViewOption> NewMinSizeOption(std::size_t minsize) = 0;
    virtual unique_ptr<MappedViewOption> NewMinTimeOption(
            const chrono::system_clock::time_point &min_time) = 0;
    virtual unique_ptr<MappedViewOption> NewMaxTimeOption(
            const chrono::system_clock::time_point &max_time) = 0;
    virtual unique_ptr<MappedViewOption> NewMinTimeFromNowOption(
            const chrono::system_clock::duration &min_time) = 0;
    virtual unique_ptr<MappedViewOption> NewMaxTimeFromNowOption(
            const chrono::system_clock::duration &max_time) = 0;
    virtual unique_ptr<MappedViewOption> NewMinTimeFromStartOption(
            const chrono::system_clock::duration &min_time) = 0;
    virtual unique_ptr<MappedViewOption> NewMaxTimeFromStartOption(
            const chrono::system_clock::duration &max_time) = 0;
  };

  unique_ptr<MappedViewWorks> NewSHMMappedViewWorks();
  unique_ptr<MappedViewWorks> NewMMFMappedViewWorks();
}

#endif
