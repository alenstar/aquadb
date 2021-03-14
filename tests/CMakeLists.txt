
set(LIB_TLV_SRC tlv_object.cpp tlv_value.cpp varint.c)

set(EXE object_test)
add_executable(${EXE} object_test.cpp)
add_dependencies(${EXE} util tlv)
target_link_libraries(${EXE} util tlv ${SYS_LIBS})
unset(EXE)

