set(Boost_NO_SYSTEM_PATHS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS
    system
    thread
    chrono
    filesystem
    program_options)

include_directories(${Boost_INCLUDE_DIRS})

add_definitions(
    -Wall
    -DBOOST_ASIO_DISABLE_BOOST_DATE_TIME=1
    -DBOOST_SPIRIT_USE_PHOENIX_V3=1
    -DBOOST_CHRONO_VERSION=2)

set(LEVELDB_NO_SYSTEM_PATHS ON)
set(LEVELDB_USE_STATIC_LIBS ON)
find_package(leveldb REQUIRED)
include_directories(${LEVELDB_INCLUDE_DIR})

add_subdirectory(memhookcore)
add_subdirectory(memhook)
add_subdirectory(memdb)
add_subdirectory(memview)
