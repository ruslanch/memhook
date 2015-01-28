add_executable(memdb
    memdb_session.cpp
    memdb_server.cpp
    memdb.cpp)

target_link_libraries(memdb
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    pthread rt)
