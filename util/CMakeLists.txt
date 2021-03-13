set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_NAME util)
# add_subdirectory(chrono)
file(GLOB_RECURSE LIB_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.cc *.cxx *.c++ *.h *.hpp)
#set(LIB_SRC_FILES )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#add_subdirectory(cron)

#include_directories(/usr/include)
#add_dependencies(${APP_NAME} ${APP_LIB_NAME})
#add_dependencies(${APP_NAME} ${ly_lib_name})
add_library(${LIB_NAME} ${LIB_SRC_FILES})

#add_dependencies(${APP_NAME} ${ly_lib_name})
#target_include_directories(${APP_NAME} PUBLIC ../src)
#install(TARGETS ${APP_NAME} RUNTIME DESTINATION bin)
