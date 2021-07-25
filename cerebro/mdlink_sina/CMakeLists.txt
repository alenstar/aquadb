set(SRC
    SinaApi.cpp
    SinaApi.h
    MdLinkMain.cpp
)


add_executable(mdlink_sina ${SRC})
add_dependencies(mdlink_sina util)
target_link_libraries( mdlink_sina 
    ${APP_LIB} 
    ${SYS_LIB}
    curl
)

