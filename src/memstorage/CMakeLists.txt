add_definitions(-fvisibility=hidden -fPIC)

add_library(memstorage STATIC
    basic_mapped_storage_creator.cpp
    mmf_mapped_storage.cpp
    mmf_mapped_storage_creator.cpp
    shm_mapped_storage.cpp
    shm_mapped_storage_creator.cpp
    network_mapped_storage.cpp
    network_mapped_storage_creator.cpp)
