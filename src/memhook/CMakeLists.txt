set(LIBUNWIND_NO_SYSTEM_PATHS ON)
set(LIBUNWIND_USE_STATIC_LIBS ON)
find_package(libunwind REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${LIBUNWIND_INCLUDE_DIR})

add_definitions(
    -DMEMHOOK_TARGET_FILE_NAME="$<TARGET_LINKER_FILE_NAME:memhook>"
    -fvisibility=hidden)

add_library(memhook SHARED
    error_code.cpp
    error_msg.cpp
    shared_memory_storage.cpp
    mapped_file_storage.cpp
    callstack.cpp
    memhook.cpp)

target_link_libraries(memhook
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBUNWIND_LIBRARIES}
    pthread rt dl)

set_target_properties(memhook
    PROPERTIES SOVERSION "1.0")
