#include <memhook/shared_memory_traits.hpp>
#include "basic_mapped_view_kit.hpp"

namespace memhook {

boost::movelib::unique_ptr<mapped_view_kit> make_shm_view_kit() {
    return boost::movelib::unique_ptr<mapped_view_kit>(new basic_mapped_view_kit<shared_memory_traits>());
}

} // namespace
