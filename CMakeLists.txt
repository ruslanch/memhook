cmake_minimum_required(VERSION 2.8)
project(memhook)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/include)

include(CheckIncludeFiles)
include(CheckCXXSourceCompiles)
include(TestBigEndian)
include(CheckLibraryExists)

test_big_endian(SYSTEM_BIG_ENDIAN)

check_include_files("sys/mman.h" HAVE_SYS_MMAN_H)
check_library_exists(rt clock_gettime   "time.h"  HAVE_CLOCK_GETTIME)
check_library_exists(dl dlopen          "dlfcn.h" HAVE_DLOPEN)

check_cxx_source_compiles("int main() { __builtin_bswap16(0); return 0; }"
    HAVE_BUILTIN_BYTESWAP_16)
check_cxx_source_compiles("int main() { __builtin_bswap32(0); return 0; }"
    HAVE_BUILTIN_BYTESWAP_32)
check_cxx_source_compiles("int main() { __builtin_bswap64(0); return 0; }"
    HAVE_BUILTIN_BYTESWAP_64)

check_cxx_source_compiles("
    #include <byteswap.h>
    int main() { __bswap_16(0); __bswap_32(0); __bswap_64(0); return 0; }
    " HAVE_GLIBC_BITS_BYTESWAP)
check_cxx_source_compiles("
    #include <byteswap.h>
    int main() { bswap_16(0); bswap_32(0); bswap_64(0); return 0; }
    " HAVE_GLIBC_BYTESWAP)

check_cxx_source_compiles("
    #include <sys/endian.h>
    int main() { swap16(0); swap32(0); swap64(0); return 0; }
    " HAVE_BSD_BYTESWAP)

check_cxx_source_compiles("
    int main() { int i = 0; __sync_val_compare_and_swap(&i, 0, 0); return 0; }
    " HAVE_GCC_SYNC_VAL_COMPARE_AND_SWAP)

check_cxx_source_compiles("
    int main() { __builtin_return_address(0); return 0; }
    " HAVE_GCC_BUILTIN_RETURN_ADDRESS)

check_cxx_source_compiles("int main() {
    __builtin_extract_return_addr(__builtin_return_address(0)); return 0; }
    " HAVE_GCC_BUILTIN_EXTRACT_RETURN_ADDRESS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/memhook/sysconfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/sysconfig.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
