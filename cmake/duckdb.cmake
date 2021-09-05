INCLUDE(ExternalProject)
#find_package(Threads)
set(CMAKE_HAVE_LIBC_PTHREAD 1)

SET(DUCKDB_SOURCES_DIR ${THIRD_PARTY_PATH}/duckdb)
SET(DUCKDB_INSTALL_DIR ${THIRD_PARTY_PATH}/install/duckdb)
SET(DUCKDB_INCLUDE_DIR "${DUCKDB_INSTALL_DIR}/include" CACHE PATH "duckdb include directory." FORCE)
SET(DUCKDB_LIBRARIES "${DUCKDB_INSTALL_DIR}/lib/libduckdb.so" CACHE FILEPATH "duckdb library." FORCE)
#SET(DUCKDB_LIBRARIES "${DUCKDB_INSTALL_DIR}/lib/libduckdb_static.a" CACHE FILEPATH "duckdb library." FORCE)
#SET(DUCKDB_DEPS_LIBRARIES "${DUCKDB_INSTALL_DIR}/lib/libfmt.a ${DUCKDB_INSTALL_DIR}/lib/libduckdb_re2.a ${DUCKDB_INSTALL_DIR}/lib/libminiz.a ${DUCKDB_INSTALL_DIR}/lib/libimdb.a ${DUCKDB_INSTALL_DIR}/lib/libpg_query.a ${DUCKDB_INSTALL_DIR}/lib/libhyperloglog.a ${DUCKDB_INSTALL_DIR}/lib/libutf8proc.a")
#LIST(APPEND DUCKDB_LIBRARIES "${DUCKDB_INSTALL_DIR}/lib/libfmt.a")
SET(DUCKDB_BINARY_DIR ${DUCKDB_INSTALL_DIR}/bin)

find_program(MAKE_EXECUTABLE
NAMES gmake mingw32-make make
NAMES_PER_DIR
DOC "GNU Make")

find_program(CMAKE_EXECUTABLE
NAMES cmake
NAMES_PER_DIR
DOC "GNU CMake")

ExternalProject_Add(
        extern_duckdb
        ${EXTERNAL_PROJECT_LOG_ARGS}
        #GIT_REPOSITORY "https://github.com/duckdb/duckdb.git"
        #GIT_TAG "v0.2.8"
        URL "https://github.com/duckdb/duckdb/archive/refs/tags/v0.2.8.tar.gz"
        PREFIX ${DUCKDB_SOURCES_DIR}
        #SOURCE_DIR ${DUCKDB_SOURCE_DIR}
        #BINARY_DIR ${DUCKDB_BINARY_DIR}
        # PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${DUCKDB_BINARY_DIR}/Makefile.inc ${_src}
        CONFIGURE_COMMAND "" 
        BUILD_IN_SOURCE 1 
        # Use make build extern project
        BUILD_COMMAND ${MAKE_EXECUTABLE} -C ${DUCKDB_SOURCES_DIR}/src/extern_duckdb VERBOSE=1
        # COMMAND ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"

        # Custom installation commands
        # INSTALL_COMMAND ""
        # Use the make installation command 
        # INSTALL_COMMAND ${MAKE_EXECUTABLE} install -C ${DUCKDB_SOURCES_DIR}/src/extern_duckdb DESTDIR=${DUCKDB_INSTALL_DIR} VERBOSE=1
        # Custom installation files and paths
        INSTALL_COMMAND mkdir -p ${DUCKDB_INSTALL_DIR}/lib/ 
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/src/libduckdb_static.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/src/libduckdb.so ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/fmt/libfmt.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/re2/libduckdb_re2.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/imdb/libimdb.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/miniz/libminiz.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/libpg_query/libpg_query.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/hyperloglog/libhyperloglog.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/build/release/third_party/utf8proc/libutf8proc.a ${DUCKDB_INSTALL_DIR}/lib/
        COMMAND mkdir -p ${DUCKDB_INCLUDE_DIR} 
        COMMAND cp -af ${DUCKDB_SOURCES_DIR}/src/extern_duckdb/src/include ${DUCKDB_INSTALL_DIR}/   
        # UPDATE_COMMAND ""
)


#ADD_LIBRARY(duckdb STATIC IMPORTED GLOBAL)
ADD_LIBRARY(duckdb SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET duckdb PROPERTY IMPORTED_LOCATION ${DUCKDB_LIBRARIES})
ADD_DEPENDENCIES(duckdb extern_duckdb)