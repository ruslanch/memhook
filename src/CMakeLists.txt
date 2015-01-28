if (MEMHOOK_USE_BOOST_PRIVATE)
    set(MEMHOOK_BOOST_NAMESPACE_PREFIX "private_")
    set(MEMHOOK_BOOST_NAMESPACE_SUFFIX "_private")
else()
    set(MEMHOOK_BOOST_NAMESPACE_PREFIX "")
    set(MEMHOOK_BOOST_NAMESPACE_SUFFIX "")
endif()

set(Boost_NO_SYSTEM_PATHS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS
    ${MEMHOOK_BOOST_NAMESPACE_PREFIX}system
    ${MEMHOOK_BOOST_NAMESPACE_PREFIX}thread
    ${MEMHOOK_BOOST_NAMESPACE_PREFIX}chrono
    ${MEMHOOK_BOOST_NAMESPACE_PREFIX}program_options
)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DMEMHOOK_BOOST_NAMESPACE=boost${MEMHOOK_BOOST_NAMESPACE_SUFFIX})

add_subdirectory(memhook)
add_subdirectory(memview)
add_subdirectory(memdb)
