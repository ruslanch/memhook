#include <memhook/mapped_file_traits.hpp>
#include "basic_mapped_view_kit.hpp"

namespace memhook {

movelib::unique_ptr<mapped_view_kit> make_mmf_view_kit() {
    return movelib::unique_ptr<mapped_view_kit>(new basic_mapped_view_kit<mapped_file_traits>());
}

} // memhook
