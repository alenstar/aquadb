#SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
#message("storage ${SUBDIRS}")
#FOREACH(subdir ${SUBDIRS})
#    #gen_unit_test( ${subdir} )
#    add_subdirectory(${subdir})
#ENDFOREACH()

set(APP_NAME kvstore)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_TLV_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#file(GLOB_RECURSE LIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/storage *.h)
file(GLOB_RECURSE LIB_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c *.cxx)
file(GLOB_RECURSE LIB_INC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h *.hpp *.hxx)

add_executable(${APP_NAME} ${LIB_SRC} ${LIB_INC})
add_dependencies(${APP_NAME} util aquadb cerebro)

target_link_libraries(${APP_NAME} util aquadb cerebro ${NURAFT_LIBRARIES} ${ROCKSDB_LIBRARIES} ${GFLAGS_LIBRARIES} ${SNAPPY_LIBRARIES} ${LZ4_LIBRARIES} ${ZLIB_LIBRARIES} ${SYS_LIBS})
# message(STATUS "LINK: ${SYS_LIBS}")




