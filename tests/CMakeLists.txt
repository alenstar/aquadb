

set(EXE object_test)
add_executable(${EXE} object_test.cpp)
add_dependencies(${EXE} util aquadb)
target_link_libraries(${EXE} util aquadb ${SYS_LIBS})
unset(EXE)

