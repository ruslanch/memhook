set(LIBUNWIND_NO_SYSTEM_PATHS ON)
set(LIBUNWIND_USE_STATIC_LIBS ON)
find_package(libunwind REQUIRED)

include_directories(${LIBUNWIND_INCLUDE_DIR})

add_definitions(
  -Wall
  -Wextra
  -DMEMHOOK_TARGET_FILE_NAME="$<TARGET_LINKER_FILE_NAME:memhook>"
  -DBOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING=1
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
  memhook.cpp
)

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/memhook.version")

set_target_properties(memhook PROPERTIES
  LINK_DEPENDS ${LINKER_SCRIPT})

target_link_libraries(memhook LINK_PRIVATE
  -Wl,--version-script=${LINKER_SCRIPT}
  -Wl,--no-whole-archive memhookcore
  -Wl,--no-whole-archive ${LIBUNWIND_LIBRARIES}
  -Wl,-Bsymbolic
  -static-libgcc
  -static-libstdc++
  ${CMAKE_THREAD_LIBS_INIT}
  rt
  dl)
