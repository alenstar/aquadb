#include_directories(./)
#add_subdirectory(hello)
#add_subdirectory(tushare)

#SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
#message("storage ${SUBDIRS}")
#FOREACH(subdir ${SUBDIRS})
#    #gen_unit_test( ${subdir} )
#    add_subdirectory(${subdir})
#ENDFOREACH()

set(LIB_TLV tlv)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_TLV_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#file(GLOB_RECURSE LIB_STORAGE_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/storage *.h)
#file(GLOB_RECURSE LIB_STORAGE_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE LIB_TLV_INC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

#add_subdirectory(src)

set(LIB_TLV_SRC tlv_object.cpp tlv_value.cpp varint.c)

add_library(${LIB_TLV} ${LIB_TLV_SRC} ${LIB_TLV_INC})
add_dependencies(${LIB_TLV} util)

#target_include_directories(${APP_NAME} PUBLIC ../src)
#install(TARGETS ${APP_NAME} RUNTIME DESTINATION bin)
