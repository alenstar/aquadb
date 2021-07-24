INCLUDE(ExternalProject)
#find_package(Threads)
set(CMAKE_HAVE_LIBC_PTHREAD 1)

SET(LZ4_SOURCES_DIR ${THIRD_PARTY_PATH}/lz4)
SET(LZ4_INSTALL_DIR ${THIRD_PARTY_PATH}/install/lz4)
SET(LZ4_INCLUDE_DIR "${LZ4_INSTALL_DIR}/include" CACHE PATH "lz4 include directory." FORCE)
SET(LZ4_LIBRARIES "${LZ4_INSTALL_DIR}/lib/liblz4.a" CACHE FILEPATH "lz4 library." FORCE)
set(LZ4_BINARY_DIR ${LZ4_INSTALL_DIR}/bin)

find_program(MAKE_EXECUTABLE
NAMES gmake mingw32-make make
NAMES_PER_DIR
DOC "GNU Make")

find_program(CMAKE_EXECUTABLE
NAMES cmake
NAMES_PER_DIR
DOC "GNU CMake")

ExternalProject_Add(
        extern_lz4
        ${EXTERNAL_PROJECT_LOG_ARGS}
        GIT_REPOSITORY "https://gitee.com/mirrors/LZ4.git"
        GIT_TAG "v1.9.3"
        PREFIX ${LZ4_SOURCES_DIR}
        #SOURCE_DIR ${LZ4_SOURCE_DIR}
        #BINARY_DIR ${LZ4_BINARY_DIR}
        # PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${LZ4_BINARY_DIR}/Makefile.inc ${_src}
        CONFIGURE_COMMAND "" 
        BUILD_IN_SOURCE 1 
        # Use make build extern project
        BUILD_COMMAND ${MAKE_EXECUTABLE} -C ${LZ4_SOURCES_DIR}/src/extern_lz4 VERBOSE=1
        # COMMAND ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"

        # Custom installation commands
        # INSTALL_COMMAND ""
        # Use the make installation command 
        # INSTALL_COMMAND ${MAKE_EXECUTABLE} install -C ${LZ4_SOURCES_DIR}/src/extern_lz4 DESTDIR=${LZ4_INSTALL_DIR} VERBOSE=1
        # Custom installation files and paths
        INSTALL_COMMAND mkdir -p ${LZ4_INSTALL_DIR}/lib/ COMMAND cp -af ${LZ4_SOURCES_DIR}/src/extern_lz4/lib/liblz4.a ${LZ4_LIBRARIES} COMMAND mkdir -p ${LZ4_INCLUDE_DIR} COMMAND cp -af ${LZ4_SOURCES_DIR}/src/extern_lz4/lib/lz4.h ${LZ4_INCLUDE_DIR}/ COMMAND cp -af ${LZ4_SOURCES_DIR}/src/extern_lz4/lib/lz4hc.h ${LZ4_INCLUDE_DIR}/ COMMAND cp -af ${LZ4_SOURCES_DIR}/src/extern_lz4/lib/lz4frame.h ${LZ4_INCLUDE_DIR}/ COMMAND cp -af ${LZ4_SOURCES_DIR}/src/extern_lz4/lib/lz4frame_static.h ${LZ4_INCLUDE_DIR}/  COMMAND cp -af ${LZ4_SOURCES_DIR}/src/extern_lz4/lib/xxhash.h ${LZ4_INCLUDE_DIR}/ 
        # UPDATE_COMMAND ""
)


#ADD_DEPENDENCIES(extern_lz4 pthreads)
ADD_LIBRARY(lz4 STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET lz4 PROPERTY IMPORTED_LOCATION ${LZ4_LIBRARIES})
ADD_DEPENDENCIES(lz4 extern_lz4)
