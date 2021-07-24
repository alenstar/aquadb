cmake_minimum_required(VERSION 3.1.12 FATAL_ERROR)
project(AquaDB C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
message("CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if($ENV{BUILD_NUMBER})
  set(BUILD_NUMBER $ENV{BUILD_NUMBER})
else()
  set(BUILD_NUMBER 0)
endif()

set(APP_NAME ${PROJECT_NAME})

# TODO OP check how this works on multiple configurations generators
# available configurations in multiple configurations enabled generators
set(CMAKE_CONFIGURATION_TYPES Debug Release)
#message("CMAKE_CFG_INTDIR = ${CMAKE_CFG_INTDIR}")

# 检查编译目录和源码是否为同一目录， 如果是则提示错误
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Do not build in-source.\nPlease remove CMakeCache.txt and the CMakeFiles/ directory.\nThen: mkdir build ; cd build ; cmake .. ; make")
endif()


if(NOT CMAKE_BUILD_TYPE)
  #set(CMAKE_BUILD_TYPE "Release" CACHE STRING
  #"Choose the type of build, options are: Debug Profile Release Asan Ubsan." FORCE)
  # default Debug
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Choose the type of build, options are: Debug Profile Release Asan Ubsan." FORCE)
endif(NOT CMAKE_BUILD_TYPE)
message("CMake build type ${CMAKE_BUILD_TYPE}")
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
    add_definitions(-DNODEBUG)
    add_definitions(-DRELEASE)
    add_definitions(-DNDEBUG)
    add_definitions(-DCOMPONENT_BUILD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2 -Wall")
else()
    # add_definitions(-DDEBUG)
    #add_definitions(-Wint-in-bool-context)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O0 -ggdb")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O0 -ggdb -Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -Wall")
endif()

# 重定义__FILE__ 去掉文件路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-builtin-macro-redefined -D__FILE__='\"$(notdir $(abspath $<))\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wno-builtin-macro-redefined -D__FILE__='\"$(notdir $(abspath $<))\"'")

execute_process(
    COMMAND bash -c "git --git-dir=${PROJECT_SOURCE_DIR}/.git rev-parse --short HEAD | tr -d '\n'"
    OUTPUT_VARIABLE BRPC_REVISION
)
set(SOFA_PBRPC_REVISION ${BRPC_REVISION})
message("project revision ${SOFA_PBRPC_REVISION}")

# for *.so
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output/lib)
# for *.a
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output/lib)
# for exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output/bin)

SET(THIRD_PARTY_PATH ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
SET(THIRD_PARTY_BUILD_TYPE Release)
SET(EXTERNAL_PROJECT_LOG_ARGS
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 0
        LOG_INSTALL 1)
#include(ExternalProject)
include(ProcessorCount)
ProcessorCount(NUM_OF_PROCESSOR)
message(NUM_OF_PROCESSOR: ${NUM_OF_PROCESSOR})


#thread
include(FindThreads)

#openssl
find_package(OpenSSL REQUIRED)

message(STATUS "ssl:" ${OPENSSL_SSL_LIBRARY})
message(STATUS "crypto:" ${OPENSSL_CRYPTO_LIBRARY})

ADD_LIBRARY(ssl SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET ssl PROPERTY IMPORTED_LOCATION ${OPENSSL_SSL_LIBRARY})

ADD_LIBRARY(crypto SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_CRYPTO_LIBRARY})

set(WITH_SYSTEM_LIBS ON)
#zlib
if (NOT WITH_SYSTEM_LIBS)
    include(zlib)
else ()
    ADD_LIBRARY(zlib SHARED IMPORTED GLOBAL)
    SET(ZLIB_LIBRARIES z)
endif ()

#lz4
include(lz4)

#gflags
if (WITH_SYSTEM_LIBS)
    find_path(GFLAGS_INCLUDE_DIR NAMES gflags/gflags.h)
    find_library(GFLAGS_LIBRARIES NAMES gflags)
    if ((NOT GFLAGS_INCLUDE_DIR) OR (NOT GFLAGS_LIBRARIES))
        message(FATAL_ERROR "Fail to find gflags")
    endif ()
    ADD_LIBRARY(gflags SHARED IMPORTED GLOBAL)
else ()
    include(gflags)
endif ()

#glog
#if (WITH_SYSTEM_LIBS)
#    find_path(GLOG_INCLUDE_DIR NAMES glog/logging.h)
#    find_library(GLOG_LIBRARIES NAMES glog)
#    if ((NOT GLOG_INCLUDE_DIR) OR (NOT GLOG_LIBRARIES))
#        message(FATAL_ERROR "Fail to find glog")
#    endif ()
#    ADD_LIBRARY(glog SHARED IMPORTED GLOBAL)
#else ()
#    include(glog)
#endif ()

#snappy
if (WITH_SYSTEM_LIBS)
    find_path(SNAPPY_INCLUDE_DIR NAMES snappy.h)
    find_library(SNAPPY_LIBRARIES NAMES snappy)
    if ((NOT SNAPPY_INCLUDE_DIR) OR (NOT SNAPPY_LIBRARIES))
        message(FATAL_ERROR "Fail to find snappy")
    endif ()
    ADD_LIBRARY(snappy SHARED IMPORTED GLOBAL)
else ()
    include(snappy)
endif ()

#protobuf
if (WITH_SYSTEM_LIBS)
    #INCLUDE(FindProtobuf)
    FIND_PACKAGE(Protobuf REQUIRED)
    ADD_LIBRARY(protobuf SHARED IMPORTED GLOBAL)
    SET_PROPERTY(TARGET protobuf PROPERTY IMPORTED_LOCATION ${PROTOBUF_LIBRARY})

    INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
    #PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER message.proto)
    #ADD_LIBRARY(proto ${PROTO_HEADER} ${PROTO_SRC})
else ()
    include(protobuf)
endif ()

#rocksdb
if (WITH_SYSTEM_LIBS)
    find_path(ROCKSDB_INCLUDE_DIR NAMES rocksdb/db.h PATHS /usr/local/thirdparty/include)
    find_library(ROCKSDB_LIBRARIES NAMES rocksdb PATHS /usr/local/thirdparty/lib)
    if ((NOT ROCKSDB_INCLUDE_DIR) OR (NOT ROCKSDB_LIBRARIES))
        message(FATAL_ERROR "Fail to find rocksdb")
    endif ()
    ADD_LIBRARY(rocksdb SHARED IMPORTED GLOBAL)
    INCLUDE_DIRECTORIES(${ROCKSDB_INCLUDE_DIR})
    message(STATUS "Found rocksdb: ${ROCKSDB_LIBRARIES} ${ROCKSDB_INCLUDE_DIR}")
else ()
    include(rocksdb)
endif ()

#brpc
#if (WITH_SYSTEM_LIBS)
#    #leveldb(for brpc)
#    find_library(LEVELDB_LIBRARIES NAMES leveldb)
#    if (NOT LEVELDB_LIBRARIES)
#        message(FATAL_ERROR "Fail to find leveldb")
#    endif ()
#
#    find_path(BRPC_INCLUDE_DIR NAMES brpc/server.h PATHS /opt/brpc/include)
#    find_library(BRPC_LIBRARIES NAMES libbrpc.a brpc PATHS /opt/brpc/lib)
#    if ((NOT BRPC_INCLUDE_DIR) OR (NOT BRPC_LIBRARIES))
#        message(FATAL_ERROR "Fail to find brpc")
#    endif ()
#    ADD_LIBRARY(brpc STATIC IMPORTED GLOBAL)
#    SET_PROPERTY(TARGET brpc PROPERTY IMPORTED_LOCATION ${BRPC_LIBRARIES})
#    INCLUDE_DIRECTORIES(${BRPC_INCLUDE_DIR})
#else ()
#    include(leveldb)
#    include(brpc)
#endif ()
#message(BRPC:${BRPC_INCLUDE_DIR}, ${BRPC_LIBRARIES})

#braft
#if (WITH_SYSTEM_LIBS)
#    find_path(BRAFT_INCLUDE_DIR NAMES braft/raft.h PATHS /opt/braft/include)
#    find_library(BRAFT_LIBRARIES NAMES libbraft.a brpc PATHS /opt/braft/lib)
#    if ((NOT BRAFT_INCLUDE_DIR) OR (NOT BRAFT_LIBRARIES))
#        message(FATAL_ERROR "Fail to find braft")
#    endif ()
#    ADD_LIBRARY(braft STATIC IMPORTED GLOBAL)
#    SET_PROPERTY(TARGET braft PROPERTY IMPORTED_LOCATION ${BRAFT_LIBRARIES})
#    INCLUDE_DIRECTORIES(${BRAFT_INCLUDE_DIR})
#else ()
##include(braft)
#endif ()
#message(braft lib : ${BRAFT_LIBRARIES})





set(PROJECT_INCS
${CMAKE_CURRENT_BINARY_DIR}
${CMAKE_CURRENT_SOURCE_DIR}
${PROJECT_BINARY_DIR}
${PROJECT_SOURCE_DIR}/thirdparty
${PROJECT_SOURCE_DIR}/thirdparty/doctest
${PROJECT_SOURCE_DIR}/db)


include_directories(${PROJECT_INCS})
link_directories(${PROJECT_SOURCE_DIR}/thirdparty/lib)

set(APP_LIBS util)

if(WIN32)
    set(SYS_LIBS ws2_32 wsock32)
else()
set(SYS_LIBS z rt dl pthread)
endif()

#add_subdirectory(thirdparty/lmdb)
add_subdirectory(db)
add_subdirectory(util)
add_subdirectory(cerebro)
add_subdirectory(tests)


# used by doc home and in packaging
#set(README_FILE ${PROJECT_SOURCE_DIR}/README.md)
# Licensing and stuff like that
#set(LICENSE_FILE ${PROJECT_SOURCE_DIR}/LICENSE-MIT)
#install(FILES
#	${README_FILE}
#	#${LICENSE_FILE}
#  DESTINATION .
#)

