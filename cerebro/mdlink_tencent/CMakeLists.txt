set(SRC
    TencentApi.cpp
    TencentApi.h
    MdLinkMain.cpp
)

add_executable(mdlink_tencent ${SRC})
add_dependencies(mdlink_tencent util)
target_link_libraries( mdlink_tencent 
    ${APP_LIB} 
    ${SYS_LIB}
    curl
)

