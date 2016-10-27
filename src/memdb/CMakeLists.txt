add_executable(memdb
    leveldb_storage.cpp
    memdb_session.cpp
    memdb_server.cpp
    memdb.cpp)

target_link_libraries(memdb
    memstorage
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${LEVELDB_LIBRARIES}
    pthread rt)
