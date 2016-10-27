#include "common.h"
#include "basic_mapped_view_factory.h"

#include <memhook/shm_mapping_traits.h>

namespace memhook
{

unique_ptr<MappedViewFactory> NewSHMMappedViewFactory()
{
    return unique_ptr<MappedViewFactory>(new BasicMappedViewFactory<SHMMappingTraits>());
}

} // namespace
