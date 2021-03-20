cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(lmdb)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if($ENV{BUILD_NUMBER})
  set(BUILD_NUMBER $ENV{BUILD_NUMBER})
else()
  set(BUILD_NUMBER 0)
endif()

set(LIB_NAME ${PROJECT_NAME})

# TODO OP check how this works on multiple configurations generators
# available configurations in multiple configurations enabled generators
set(CMAKE_CONFIGURATION_TYPES Debug Release)
#message("CMAKE_CFG_INTDIR = ${CMAKE_CFG_INTDIR}")

# 检查编译目录和源码是否为同一目录， 如果是则提示错误
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Do not build in-source.\nPlease remove CMakeCache.txt and the CMakeFiles/ directory.\nThen: mkdir build ; cd build ; cmake .. ; make")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Release" )
    add_definitions(-DNODEBUG)
else()
    add_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O0 -ggdb")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O0 -ggdb")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE LIB_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.cc *.cxx *.c++ *.c *.h *.hpp)
#set(LIB_SRC_FILES )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${LIB_NAME} STATIC ${LIB_SRC_FILES})

if (LMDB_TEST_ENABLED)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
add_executable(lmdb_example example/example.cpp ${LIB_NAME})
add_dependencies(lmdb_example ${LIB_NAME})
target_link_libraries(lmdb_example ${LIB_NAME} pthread)
endif()

