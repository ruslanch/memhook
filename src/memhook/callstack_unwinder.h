#ifndef MEMHOOK_SRC_MEMHOOK_CALLSTACK_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_CALLSTACK_H_INCLUDED

#include "common.h"

#include <memhook/callstack.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace memhook {
  class CallStackUnwinder : noncopyable {
  public:
    void Initialize();
    void Destroy();

    void GetCallStackInfo(CallStackInfo &callstack, size_t skip_frames, bool need_unwind_proc_info);
    void GetCallStackUnwindProcInfo(CallStackInfo &callstack);
    void FlushCallStackCache();

  private:
    void CheckImpl();

    class Impl;

    typedef shared_ptr<Impl> ImplPtrBase;

    struct ImplPtr : ImplPtrBase {
      ImplPtr();
      ~ImplPtr();
    } m_impl;
  };

}  // ns memhook

#endif
