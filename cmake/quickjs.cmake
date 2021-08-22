INCLUDE(ExternalProject)
#find_package(Threads)
set(CMAKE_HAVE_LIBC_PTHREAD 1)

SET(QUICKJS_SOURCES_DIR ${THIRD_PARTY_PATH}/quickjs)
SET(QUICKJS_INSTALL_DIR ${THIRD_PARTY_PATH}/install/quickjs)
SET(QUICKJS_INCLUDE_DIR "${QUICKJS_INSTALL_DIR}/include" CACHE PATH "quickjs include directory." FORCE)
SET(QUICKJS_LIBRARIES "${QUICKJS_INSTALL_DIR}/lib/libquickjs.a" CACHE FILEPATH "quickjs library." FORCE)
set(QUICKJS_BINARY_DIR ${QUICKJS_INSTALL_DIR}/bin)

find_program(MAKE_EXECUTABLE
NAMES gmake mingw32-make make
NAMES_PER_DIR
DOC "GNU Make")

find_program(CMAKE_EXECUTABLE
NAMES cmake
NAMES_PER_DIR
DOC "GNU CMake")

ExternalProject_Add(
        extern_quickjs
        ${EXTERNAL_PROJECT_LOG_ARGS}
        #GIT_REPOSITORY "https://gitee.com/mirrors/QuickJS.git"
        #GIT_TAG "20200705"
        URL "https://bellard.org/quickjs/quickjs-2021-03-27.tar.xz"
        PREFIX ${QUICKJS_SOURCES_DIR}
        #SOURCE_DIR ${QUICKJS_SOURCE_DIR}
        #BINARY_DIR ${QUICKJS_BINARY_DIR}
        # PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${QUICKJS_BINARY_DIR}/Makefile.inc ${_src}
        CONFIGURE_COMMAND "" 
        BUILD_IN_SOURCE 1 
        # Use make build extern project
        BUILD_COMMAND ${MAKE_EXECUTABLE} -C ${QUICKJS_SOURCES_DIR}/src/extern_quickjs VERBOSE=1
        # COMMAND ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"

        # Custom installation commands
        # INSTALL_COMMAND ""
        # Use the make installation command 
        # INSTALL_COMMAND ${MAKE_EXECUTABLE} install -C ${QUICKJS_SOURCES_DIR}/src/extern_quickjs DESTDIR=${QUICKJS_INSTALL_DIR} VERBOSE=1
        # Custom installation files and paths
        INSTALL_COMMAND mkdir -p ${QUICKJS_INSTALL_DIR}/lib/ COMMAND cp -af ${QUICKJS_SOURCES_DIR}/src/extern_quickjs/libquickjs.a ${QUICKJS_LIBRARIES} COMMAND mkdir -p ${QUICKJS_INCLUDE_DIR} COMMAND cp -af ${QUICKJS_SOURCES_DIR}/src/extern_quickjs/quickjs.h ${QUICKJS_INCLUDE_DIR}/ COMMAND cp -af ${QUICKJS_SOURCES_DIR}/src/extern_quickjs/quickjs-libc.h ${QUICKJS_INCLUDE_DIR}/ COMMAND cp -af ${QUICKJS_SOURCES_DIR}/src/extern_quickjs/quickjs-opcode.h ${QUICKJS_INCLUDE_DIR}/ COMMAND cp -af ${QUICKJS_SOURCES_DIR}/src/extern_quickjs/quickjs-atom.h ${QUICKJS_INCLUDE_DIR}/ 
        # UPDATE_COMMAND ""
)


ADD_LIBRARY(quickjs STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET quickjs PROPERTY IMPORTED_LOCATION ${QUICKJS_LIBRARIES})
ADD_DEPENDENCIES(quickjs extern_quickjs)
