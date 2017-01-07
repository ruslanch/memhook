#include "common.h"
#include "basic_mapped_view_works.h"

#include <memhook/mmf_mapping_traits.h>

namespace memhook {
  unique_ptr<MappedViewWorks> NewMMFMappedViewWorks() {
    return unique_ptr<MappedViewWorks>(new BasicMappedViewWorks<MMFMappingTraits>());
  }
}
