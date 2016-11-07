#ifndef MEMHOOK_SRC_MEMSTORAGE_SHM_MAPPED_STORAGE_H_INCLUDED
#define MEMHOOK_SRC_MEMSTORAGE_SHM_MAPPED_STORAGE_H_INCLUDED

#include "common.h"
#include "basic_mapped_storage.h"

#include <memhook/shm_mapping_traits.h>

namespace memhook
{

typedef BasicMappedStorage<
        SHMMappingTraits
    > SHMMappedStorage;

} // memhook

#endif
