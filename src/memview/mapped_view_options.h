#ifndef MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_OPTIONS_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_OPTIONS_H_INCLUDED

#include "common.h"

#include <memhook/mapping_traits.h>

#include <boost/chrono/system_clocks.hpp>

namespace memhook
{

class MappedViewOption : noncopyable
{
public:
    virtual ~MappedViewOption() {}
    virtual bool Invoke(const TraceInfoBase &tinfo) const = 0;
};

class MinSizeMappedViewOption : public MappedViewOption
{
    std::size_t min_size;
public:
    MinSizeMappedViewOption(std::size_t min_size) : min_size(min_size)
    {}

    bool Invoke(const TraceInfoBase &tinfo) const
    {
        return tinfo.memsize >= min_size;
    }
};

class MinTimeMappedViewOption : public MappedViewOption
{
    boost::chrono::system_clock::time_point min_time;
public:
    MinTimeMappedViewOption(const boost::chrono::system_clock::time_point &min_time)
        : min_time(min_time)
    {}

    bool Invoke(const TraceInfoBase &tinfo) const
    {
        return tinfo.timestamp >= min_time;
    }
};

class MaxTimeMappedViewOption : public MappedViewOption
{
    boost::chrono::system_clock::time_point max_time;
public:
    MaxTimeMappedViewOption(const boost::chrono::system_clock::time_point &max_time)
        : max_time(max_time)
    {}

    bool Invoke(const TraceInfoBase &tinfo) const
    {
        return tinfo.timestamp <= max_time;
    }
};

class MinTimeFromNowMappedViewOption : public MappedViewOption
{
    boost::chrono::system_clock::time_point current_time;
    boost::chrono::system_clock::duration   min_duration;
public:
    MinTimeFromNowMappedViewOption(const boost::chrono::system_clock::duration &min_duration)
            : current_time(boost::chrono::system_clock::now())
            , min_duration(min_duration)
    {}

    bool Invoke(const TraceInfoBase &tinfo) const
    {
        return (current_time - tinfo.timestamp) >= min_duration;
    }
};

class MaxTimeFromNowMappedViewOption : public MappedViewOption
{
    boost::chrono::system_clock::time_point current_time;
    boost::chrono::system_clock::duration   max_duration;
public:
    MaxTimeFromNowMappedViewOption(const boost::chrono::system_clock::duration &max_duration)
            : current_time(boost::chrono::system_clock::now())
            , max_duration(max_duration)
    {}

    bool Invoke(const TraceInfoBase &tinfo) const
    {
        return (current_time - tinfo.timestamp) <= max_duration;
    }
};

} // namespace

#endif
