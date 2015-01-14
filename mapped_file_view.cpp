#include "mapped_file_view.hpp"

namespace memhook {

mapped_view_kit *make_mmf_view_kit() {
    return new mapped_file_view_kit();
}

} // memhook
