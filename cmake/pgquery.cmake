INCLUDE(ExternalProject)
#find_package(Threads)
set(CMAKE_HAVE_LIBC_PTHREAD 1)

SET(PGQUERY_SOURCES_DIR ${THIRD_PARTY_PATH}/pgquery)
SET(PGQUERY_INSTALL_DIR ${THIRD_PARTY_PATH}/install/pgquery)
SET(PGQUERY_INCLUDE_DIR "${PGQUERY_INSTALL_DIR}/include" CACHE PATH "pgquery include directory." FORCE)
SET(PGQUERY_LIBRARIES "${PGQUERY_INSTALL_DIR}/lib/libpg_query.a" CACHE FILEPATH "pgquery library." FORCE)
set(PGQUERY_BINARY_DIR ${PGQUERY_INSTALL_DIR}/bin)

find_program(MAKE_EXECUTABLE
NAMES gmake mingw32-make make
NAMES_PER_DIR
DOC "GNU Make")

find_program(CMAKE_EXECUTABLE
NAMES cmake
NAMES_PER_DIR
DOC "GNU CMake")

ExternalProject_Add(
        extern_pgquery
        ${EXTERNAL_PROJECT_LOG_ARGS}
        GIT_REPOSITORY "https://gitee.com/mirrors_jirutka/libpg_query.git"
        GIT_TAG "13-2.0.4"
        PREFIX ${PGQUERY_SOURCES_DIR}
        #SOURCE_DIR ${PGQUERY_SOURCE_DIR}
        #BINARY_DIR ${PGQUERY_BINARY_DIR}
        # PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${PGQUERY_BINARY_DIR}/Makefile.inc ${_src}
        CONFIGURE_COMMAND "" 
        BUILD_IN_SOURCE 1 
        # Use make build extern project
        BUILD_COMMAND ${MAKE_EXECUTABLE} -C ${PGQUERY_SOURCES_DIR}/src/extern_pgquery VERBOSE=1
        # COMMAND ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"

        # Custom installation commands
        # INSTALL_COMMAND ""
        # Use the make installation command 
        # INSTALL_COMMAND ${MAKE_EXECUTABLE} install -C ${PGQUERY_SOURCES_DIR}/src/extern_pgquery DESTDIR=${PGQUERY_INSTALL_DIR} VERBOSE=1
        # Custom installation files and paths
        INSTALL_COMMAND mkdir -p ${PGQUERY_INSTALL_DIR}/lib/ COMMAND cp -af ${PGQUERY_SOURCES_DIR}/src/extern_pgquery/lib/libpg_query.a ${PGQUERY_LIBRARIES} 
		COMMAND mkdir -p ${PGQUERY_INCLUDE_DIR} COMMAND cp -af ${PGQUERY_SOURCES_DIR}/src/extern_pgquery/lib/pg_query.h ${PGQUERY_INCLUDE_DIR}/ 
        # UPDATE_COMMAND ""
)


#ADD_DEPENDENCIES(extern_pgquery pthreads)
ADD_LIBRARY(pgquery STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET pgquery PROPERTY IMPORTED_LOCATION ${PGQUERY_LIBRARIES})
ADD_DEPENDENCIES(pgquery extern_pgquery)
