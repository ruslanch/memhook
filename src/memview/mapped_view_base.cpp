#include "mapped_view_base.h"
#include "mapped_view_options.h"

#include <boost/bind.hpp>
#include <boost/range/algorithm.hpp>

#include <cxxabi.h>
#include <signal.h>

namespace memhook
{
namespace mapped_view_detail
{
    static bool volatile is_interrupted = false;

    void InterruptHandler(int, siginfo_t *, void *)
    {
        is_interrupted = true;
    }
} // mapped_view_detail

void UniqueCharBufNoFree(const char *) BOOST_NOEXCEPT_OR_NOTHROW
{
    // nothing
}

void UniqueCharBufFree(const char *mem) BOOST_NOEXCEPT_OR_NOTHROW
{
    free(const_cast<char *>(mem));
}

UniqueCharBuf CxxSymbolDemangle(const char *source)
{
    int ret = 0;
    UniqueCharBuf res(abi::__cxa_demangle(source, NULL, NULL, &ret), UniqueCharBufFree);
    if (ret == 0 && res)
        return boost::move(res);
    return UniqueCharBuf(source, UniqueCharBufNoFree);
}

MappedViewBase::MappedViewBase()
    : options_()
    , flags_(SortByTime)
{}

bool MappedViewBase::GetOptionFlag(OptionFlag option_flag) const
{
    return !!(flags_ & option_flag);
}

void MappedViewBase::SetOptionFlag(OptionFlag option_flag, bool setval)
{
    const uint32_t sortmask = 0x00f0;
    if (option_flag & sortmask)
        flags_ &= ~sortmask;

    if (setval)
        flags_ |= option_flag;
    else
        flags_ &= ~option_flag;
}

void MappedViewBase::SetOption(BOOST_RV_REF(unique_ptr<MappedViewOption>) option)
{
    options_.push_back(option.get());
    option.release();
}

bool MappedViewBase::CheckTraceInfoOptions(const TraceInfoBase &tinfo) const
{
    return boost::find_if(options_,
        !boost::bind(&MappedViewOption::Invoke, _1, boost::cref(tinfo))) == options_.end();
}

bool MappedViewBase::IsInterrupted()
{
    return mapped_view_detail::is_interrupted;
}

} // memhook
