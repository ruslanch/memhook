#include "no_hook.h"

namespace memhook {
  __thread ssize_t NoHook::hook_depth_ = 0;
}
