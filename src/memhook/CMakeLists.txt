set(LIBUNWIND_NO_SYSTEM_PATHS ON)
set(LIBUNWIND_USE_STATIC_LIBS ON)
find_package(libunwind REQUIRED)

include_directories(${LIBUNWIND_INCLUDE_DIR})

add_definitions(
  -Wall
  -Wextra
  -DMEMHOOK_TARGET_FILE_NAME="$<TARGET_LINKER_FILE_NAME:memhook>"
  -DPIC
  -D_REENTRANT=1
  -fvisibility=hidden
  -fno-builtin-malloc
  -fno-builtin-free
  -fno-builtin-realloc
  -fno-builtin-calloc
  -fno-builtin-cfree
  -fno-builtin-memalign
  -fno-builtin-posix_memalign
  -fno-builtin-valloc
  -fno-builtin-pvalloc)

add_library(memhook SHARED
  boost_chrono.cpp
  boost_error_code.cpp
  static_buf_alloc.cpp
  log.cpp
  glibc.cpp
  dl_functions.cpp
  no_hook.cpp
  spin_lock.cpp
  thread.cpp
  elf_injection.cpp
  callstack_unwinder.cpp
  engine.cpp
  memhook.cpp)

target_link_libraries(memhook LINK_PRIVATE
  -Wl,--no-whole-archive memhookcore
  -Wl,--no-whole-archive ${LIBUNWIND_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  rt
  dl
  -Wl,-Bsymbolic)
