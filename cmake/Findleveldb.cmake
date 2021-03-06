# LEVELDB_FOUND       - True if leveldb was found
# LEVELDB_LIBRARIES   - The libraries needed to use leveldb
# LEVELDB_INCLUDE_DIR - Location of leveldb/db.h

SET(LEVELDB_SEARCH_DIRS ${LEVELDB_ROOT})
IF(LEVELDB_NO_SYSTEM_PATHS)
    LIST(APPEND LEVELDB_SEARCH_DIRS NO_CMAKE_SYSTEM_PATH)
ENDIF()

IF(LEVELDB_USE_STATIC_LIBS)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
ENDIF()

FIND_PATH(LEVELDB_INCLUDE_DIR
    leveldb/db.h
    PATH_SUFFIXES "include"
    HINTS ${LEVELDB_SEARCH_DIRS})

IF(NOT LEVELDB_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "failed to find leveldb/db.h")
ELSEIF(NOT EXISTS "${LEVELDB_INCLUDE_DIR}/leveldb/db.h")
    MESSAGE(FATAL_ERROR "leveldb/db.h was found, but leveldb/db.h was not found in that directory")
    SET(LEVELDB_INCLUDE_DIR "")
ENDIF()

FIND_LIBRARY(LEVELDB_GENERIC_LIBRARY
    "leveldb"
    PATH_SUFFIXES "out-shared" "out-static"
    HINTS ${LEVELDB_SEARCH_DIRS})
IF(NOT LEVELDB_GENERIC_LIBRARY)
    MESSAGE(FATAL_ERROR "failed to find leveldb generic library")
ENDIF()
SET(LEVELDB_LIBRARIES ${LEVELDB_GENERIC_LIBRARY})

MARK_AS_ADVANCED(LEVELDB_LIBRARIES LEVELDB_INCLUDE_DIR)
