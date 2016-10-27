#ifndef MEMHOOK_SRC_MEMHOOK_CALLSTACK_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_CALLSTACK_H_INCLUDED

#include "common.h"

#include <memhook/callstack.h>

#include <boost/move/unique_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace memhook
{

class CallStackUnwinder : noncopyable
{
public:
    void Initialize();
    void Destroy();

    void GetCallStackInfo(CallStackInfo &callstack, size_t skip_frames);
    void FlushCallStackCache();

private:
    class Impl;

    typedef boost::movelib::unique_ptr<Impl> ImplUniquePtr;

    struct ImplPtr : ImplUniquePtr
    {
        ImplPtr();
        explicit ImplPtr(BOOST_RV_REF(ImplUniquePtr) p);
        ~ImplPtr();
    } impl_;
};

} // ns memhook

#endif
