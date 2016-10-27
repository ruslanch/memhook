#include "common.h"
#include "basic_mapped_view_factory.h"

#include <memhook/mmf_mapping_traits.h>

namespace memhook
{

unique_ptr<MappedViewFactory> NewMMFMappedViewFactory()
{
    return unique_ptr<MappedViewFactory>(new BasicMappedViewFactory<MMFMappingTraits>());
}

} // memhook
