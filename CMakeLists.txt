cmake_minimum_required(VERSION 2.8)
project(memhook)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(Boost_NO_SYSTEM_PATHS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS
    system
    thread
    chrono
    program_options
)

set(LIBUNWIND_NO_SYSTEM_PATHS ON)
set(LIBUNWIND_USE_STATIC_LIBS ON)
find_package(libunwind REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${LIBUNWIND_INCLUDE_DIR})

add_definitions(
    -fvisibility=hidden)

add_library(${PROJECT_NAME} SHARED
    error_code.cpp
    shared_memory_storage.cpp
    mapped_file_storage.cpp
    callstack.cpp
    memhook.cpp)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBUNWIND_LIBRARIES}
    pthread rt dl)

set_target_properties(${PROJECT_NAME}
    PROPERTIES SOVERSION "1.0")

add_executable(memview
    basic_mapped_view.cpp
    mapped_file_view.cpp
    shared_memory_view.cpp
    interprocess_scoped_lock.cpp
    memview.cpp)

target_link_libraries(memview
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    pthread rt)
