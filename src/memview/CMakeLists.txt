add_executable(memview
    mapped_view_base.cpp
    mapped_file_view.cpp
    shared_memory_view.cpp
    interprocess_scoped_lock.cpp
    memview.cpp)

target_link_libraries(memview
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    pthread rt)
