#include_directories(./)
#add_subdirectory(hello)
#add_subdirectory(tushare)

#SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
#message("storage ${SUBDIRS}")
#FOREACH(subdir ${SUBDIRS})
#    #gen_unit_test( ${subdir} )
#    add_subdirectory(${subdir})
#ENDFOREACH()

set(LIB_NAME tdb)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_TLV_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#file(GLOB_RECURSE LIB_STORAGE_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/storage *.h)
#file(GLOB_RECURSE LIB_STORAGE_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE LIB_INC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

#add_subdirectory(src)

set(LIB_SRC tdb.cpp tbl.cpp)

add_library(${LIB_NAME} ${LIB_SRC} ${LIB_INC})
add_dependencies(${LIB_NAME} util lmdb)

set(APP_NAME tdb_example)
add_executable(${APP_NAME} example.cpp)
add_dependencies(${APP_NAME} ${LIB_NAME})
target_link_libraries(${APP_NAME} ${LIB_NAME} lmdb util db ${SYS_LIBS})
message("LINK: ${SYS_LIBS}")

#target_include_directories(${APP_NAME} PUBLIC ../src)
#install(TARGETS ${APP_NAME} RUNTIME DESTINATION bin)
