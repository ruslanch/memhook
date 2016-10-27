set(LIBUNWIND_NO_SYSTEM_PATHS ON)
set(LIBUNWIND_USE_STATIC_LIBS ON)
find_package(libunwind REQUIRED)

include_directories(${LIBUNWIND_INCLUDE_DIR})

add_definitions(
    -DMEMHOOK_TARGET_FILE_NAME="$<TARGET_LINKER_FILE_NAME:memhook>"
    -fvisibility=hidden)

add_library(memhook SHARED
    boost_error_code.cpp
    boost_chrono.cpp
    static_buf_alloc.cpp
    dl_functions.cpp
    dl_fcn_hook.cpp
    no_hook.cpp
    thread.cpp
    utils.cpp
    callstack_unwinder.cpp
    engine.cpp
    memhook.cpp
    ../memstorage/mmf_mapped_storage.cpp
    ../memstorage/shm_mapped_storage.cpp
    ../memstorage/network_mapped_storage.cpp)

target_link_libraries(memhook LINK_PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBUNWIND_LIBRARIES}
    pthread rt dl)

set_target_properties(memhook
    PROPERTIES SOVERSION "1.0")
