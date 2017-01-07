#include "mapped_view_base.h"
#include "mapped_view_options.h"

#include <boost/bind.hpp>
#include <boost/range/algorithm.hpp>

#include <signal.h>

namespace memhook {
  namespace mapped_view_detail {
    static bool volatile g_is_interrupted = false;

    void InterruptHandler(int, siginfo_t *, void *) {
      g_is_interrupted = true;
    }
  }

  MappedViewBase::MappedViewBase()
      : m_options()
      , m_flags(kSortByTime) {}

  bool MappedViewBase::GetOptionFlag(OptionFlag option_flag) const {
    return !!(m_flags & option_flag);
  }

  void MappedViewBase::SetOptionFlag(OptionFlag option_flag, bool setval) {
    const uint32_t sortmask = 0x00f0;
    if (option_flag & sortmask)
      m_flags &= ~sortmask;

    if (setval)
      m_flags |= option_flag;
    else
      m_flags &= ~option_flag;
  }

  void MappedViewBase::SetOption(BOOST_RV_REF(unique_ptr<MappedViewOption>) option) {
    m_options.push_back(option.get());
    option.release();
  }

  bool MappedViewBase::CheckTraceInfoOptions(const TraceInfoBase &tinfo) const {
    return (boost::find_if(m_options,
            !boost::bind(&MappedViewOption::Call, _1, boost::cref(tinfo))) == m_options.end());
  }

  bool MappedViewBase::IsInterrupted() {
    return mapped_view_detail::g_is_interrupted;
  }
}
