#include_directories(./)
#add_subdirectory(hello)
#add_subdirectory(tushare)

#SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
#message("storage ${SUBDIRS}")
#FOREACH(subdir ${SUBDIRS})
#    #gen_unit_test( ${subdir} )
#    add_subdirectory(${subdir})
#ENDFOREACH()

set(LIB_NAME aquadb)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_TLV_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#file(GLOB_RECURSE LIB_STORAGE_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/storage *.h)
#file(GLOB_RECURSE LIB_STORAGE_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE LIB_INC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h *.hpp *.hxx)

set(LIB_BASE_SRC tuple_value.cpp varint.c mut_table_key.cpp table_key.cpp tuple_record.cpp str.cpp descriptor.cpp rocks_wrapper.cpp)

#add_subdirectory(src)

set(LIB_SRC db.cpp table.cpp raft_log_store.cpp)

add_library(${LIB_NAME} ${LIB_BASE_SRC} ${LIB_SRC} ${LIB_INC})
add_dependencies(${LIB_NAME} util)

set(APP_NAME db_example)
add_executable(${APP_NAME} example.cpp)
add_dependencies(${APP_NAME} ${LIB_NAME})
target_link_libraries(${APP_NAME} ${LIB_NAME} util ${ROCKSDB_LIBRARIES} ${GFLAGS_LIBRARIES} ${SNAPPY_LIBRARIES} ${LZ4_LIBRARIES} ${ZLIB_LIBRARIES} ${SYS_LIBS})
# message("LINK: ${SYS_LIBS}")

#target_include_directories(${APP_NAME} PUBLIC ../src)
#install(TARGETS ${APP_NAME} RUNTIME DESTINATION bin)



#find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
#find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
#if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
#    message(FATAL_ERROR "Fail to find gflags")
#endif()
#include_directories(${GFLAGS_INCLUDE_PATH})
#
#execute_process(
#    COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
#    OUTPUT_VARIABLE GFLAGS_NS
#)
#if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
#    execute_process(
#        COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
#        OUTPUT_VARIABLE GFLAGS_NS
#    )
#endif()





