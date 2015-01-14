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
