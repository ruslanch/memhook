#ifndef MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_BASE_H_INCLUDED
#define MEMHOOK_SRC_MEMVIEW_MAPPED_VIEW_BASE_H_INCLUDED

#include "common.h"
#include "mapped_view.h"

#include <memhook/mapping_traits.h>

#include <boost/ptr_container/ptr_vector.hpp>

namespace memhook
{

typedef boost::movelib::unique_ptr<const char, void (*)(const char *)> UniqueCharBuf;
void UniqueCharBufNoFree(const char *) BOOST_NOEXCEPT_OR_NOTHROW;
void UniqueCharBufFree(const char *) BOOST_NOEXCEPT_OR_NOTHROW;
UniqueCharBuf CxxSymbolDemangle(const char *source);

class MappedViewBase : public MappedView
{
public:
    MappedViewBase();

    bool GetOptionFlag(OptionFlag option_flag) const;
    void SetOptionFlag(OptionFlag option_flag, bool setval);

    void SetOption(BOOST_RV_REF(unique_ptr<MappedViewOption>) option);
    bool CheckTraceInfoOptions(const TraceInfoBase &tinfo) const;

    virtual UniqueCharBuf GetModulePath(uintptr_t shl_addr) const = 0;
    virtual UniqueCharBuf GetProcName(uintptr_t ip) const = 0;

    static bool IsInterrupted();

private:
    boost::ptr_vector<MappedViewOption> options_;
    uint32_t flags_;
};

} // memhook

#endif
