#include "no_hook.hpp"

namespace memhook {

__thread ssize_t NoHook::hook_depth_ = 0;

} // ns memhook
