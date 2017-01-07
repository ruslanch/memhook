#include "common.h"
#include "basic_mapped_view_works.h"

#include <memhook/shm_mapping_traits.h>

namespace memhook {
  unique_ptr<MappedViewWorks> NewSHMMappedViewWorks() {
    return unique_ptr<MappedViewWorks>(new BasicMappedViewWorks<SHMMappingTraits>());
  }
}
