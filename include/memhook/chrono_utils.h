#ifndef MEMHOOK_INCLUDE_CHRONO_UTILS_H_INCLUDED
#define MEMHOOK_INCLUDE_CHRONO_UTILS_H_INCLUDED

#include <memhook/common.h>

#include <boost/chrono/time_point.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/chrono/chrono_io.hpp>

namespace memhook {
  template  <typename Clock, typename Duration>
  void ChronoTimePointFromString(const std::string &string,
          boost::chrono::time_point<Clock, Duration> &time_point) {
    std::istringstream sstream(string);
    sstream >> boost::chrono::time_fmt(boost::chrono::timezone::local, "%Y-%m-%d %H:%M:%S")
            >> time_point;
  }

  template  <typename Rep, typename Period>
  void ChronoDurationFromString(const std::string &string,
          boost::chrono::duration<Rep, Period> &duration) {
    std::istringstream sstream(string);
    sstream >> duration;
  }
}
#endif
