add_definitions(
  -Wall
  -Wextra
  -fvisibility=hidden
  -fPIC
  -DPIC
  -D_REENTRANT=1)

add_library(memhookcore STATIC
    scoped_signal_lock.cpp
    scoped_signal.cpp
    file_based_mapped_storage_helpers.cpp
    mmf_mapped_storage.cpp
    shm_mapped_storage.cpp
    network_mapped_storage.cpp
    leveldb_storage.cpp)
