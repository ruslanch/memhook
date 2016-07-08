#ifndef MEMHOOK_SRC_MEMHOOK_NO_HOOK_HPP_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_NO_HOOK_HPP_INCLUDED

#include <memhook/common.hpp>
#include <cstddef>

namespace memhook {

class NoHook {
public:
  NoHook()  { ++hook_depth_; }
  ~NoHook() { --hook_depth_; }

  static bool IsNested() {
    return hook_depth_ != 0;
  }

private:
  static __thread ssize_t hook_depth_;
};

} // ns memhook


#endif
