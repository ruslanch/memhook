#ifndef MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_OPTIONS_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_OPTIONS_H_INCLUDED

#include "common.h"
#include <memhook/mapping_traits.h>

namespace memhook {
  class MappedViewOption : noncopyable {
  public:
    virtual ~MappedViewOption() {}
    virtual bool Call(const TraceInfoBase &tinfo) const = 0;
  };

  class MinSizeMappedViewOption : public MappedViewOption {
    std::size_t m_min_size;
  public:
    MinSizeMappedViewOption(std::size_t min_size)
        : m_min_size(min_size) {}

    bool Call(const TraceInfoBase &tinfo) const {
      return tinfo.memsize >= m_min_size;
    }
  };

  class MinTimeMappedViewOption : public MappedViewOption {
    chrono::system_clock::time_point m_min_time;
  public:
    MinTimeMappedViewOption(const chrono::system_clock::time_point &min_time)
        : m_min_time(min_time) {}

    bool Call(const TraceInfoBase &tinfo) const {
      return tinfo.timestamp >= m_min_time;
    }
  };

  class MaxTimeMappedViewOption : public MappedViewOption {
    chrono::system_clock::time_point m_max_time;
  public:
    MaxTimeMappedViewOption(const chrono::system_clock::time_point &max_time)
        : m_max_time(max_time) {}

    bool Call(const TraceInfoBase &tinfo) const {
      return tinfo.timestamp <= m_max_time;
    }
  };

  class MinTimeFromNowMappedViewOption : public MappedViewOption {
    chrono::system_clock::time_point m_current_time;
    chrono::system_clock::duration   m_min_duration;
  public:
    MinTimeFromNowMappedViewOption(const chrono::system_clock::duration &min_duration)
        : m_current_time(chrono::system_clock::now())
        , m_min_duration(min_duration) {}

    bool Call(const TraceInfoBase &tinfo) const {
      return (m_current_time - tinfo.timestamp) >= m_min_duration;
    }
  };

  class MaxTimeFromNowMappedViewOption : public MappedViewOption {
    chrono::system_clock::time_point m_current_time;
    chrono::system_clock::duration   m_max_duration;
  public:
    MaxTimeFromNowMappedViewOption(const chrono::system_clock::duration &max_duration)
        : m_current_time(chrono::system_clock::now()), m_max_duration(max_duration) {}

    bool Call(const TraceInfoBase &tinfo) const {
      return (m_current_time - tinfo.timestamp) <= m_max_duration;
    }
  };

  class MinTimeFromStartMappedViewOption : public MappedViewOption {
    mutable chrono::system_clock::time_point m_start_time;
    chrono::system_clock::duration           m_min_duration;
  public:
    MinTimeFromStartMappedViewOption(const chrono::system_clock::duration &min_duration)
        : m_start_time(), m_min_duration(min_duration) {}

    bool Call(const TraceInfoBase &tinfo) const {
      if (BOOST_UNLIKELY(m_start_time == chrono::system_clock::time_point()))
        m_start_time = tinfo.timestamp;
      return (tinfo.timestamp - m_start_time) >= m_min_duration;
    }
  };

  class MaxTimeFromStartMappedViewOption : public MappedViewOption {
    mutable chrono::system_clock::time_point m_start_time;
    chrono::system_clock::duration           m_max_duration;
  public:
    MaxTimeFromStartMappedViewOption(const chrono::system_clock::duration &max_duration)
        : m_start_time(), m_max_duration(max_duration) {}

    bool Call(const TraceInfoBase &tinfo) const {
      if (BOOST_UNLIKELY(m_start_time == chrono::system_clock::time_point()))
        m_start_time = tinfo.timestamp;
      return (tinfo.timestamp - m_start_time) <= m_max_duration;
    }
  };

} // namespace

#endif
