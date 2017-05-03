add_definitions(
  -Wall
  -Wextra
  -D_REENTRANT=1)

add_executable(memdb
  memdb_session.cpp
  memdb_server.cpp
  memdb.cpp)

target_link_libraries(memdb
  -Wl,--no-whole-archive memhookcore
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  rt)
