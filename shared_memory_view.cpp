#include "shared_memory_view.hpp"

namespace memhook {

mapped_view_kit *make_shm_view_kit() {
    return new shared_memory_view_kit();
}

} // namespace
