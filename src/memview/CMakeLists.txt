add_definitions(
  -Wall
  -Wextra
  -D_REENTRANT=1)

add_executable(memview
  mapped_view_helpers.cpp
  mapped_view_base.cpp
  mmf_mapped_view.cpp
  shm_mapped_view.cpp
  interprocess_scoped_lock.cpp
  memview.cpp)

target_link_libraries(memview
  -Wl,--no-whole-archive memhookcore
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  rt)
