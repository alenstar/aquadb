cmake_minimum_required(VERSION 3.1.12 FATAL_ERROR)
project(tlv_box)
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


set(PROJECT_INCS
${CMAKE_CURRENT_BINARY_DIR}
${CMAKE_CURRENT_SOURCE_DIR}
${PROJECT_BINARY_DIR}
${PROJECT_SOURCE_DIR}/thirdparty
${PROJECT_SOURCE_DIR}/thirdparty/doctest
${PROJECT_SOURCE_DIR}/thirdparty/lmdb
${PROJECT_SOURCE_DIR}/tlv)


include_directories(${PROJECT_INCS})
link_directories(${PROJECT_SOURCE_DIR}/thirdparty/lib)

set(APP_LIBS util)

if(WIN32)
    set(SYS_LIBS ws2_32 wsock32)
else()
set(SYS_LIBS z rt dl pthread)
endif()

add_subdirectory(thirdparty/lmdb)
add_subdirectory(tdb)
add_subdirectory(util)
add_subdirectory(tlv)
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

