cmake_minimum_required(VERSION 3.0)

project(melon_test)

include(ExternalProject)

##### melon
ExternalProject_Add(melon PREFIX melon
    URL ${CMAKE_CURRENT_SOURCE_DIR}/melon-f9a4563.zip
    URL_MD5 e3a83f4a2fb2c716c003d43a9b38e84d
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --melang-prefix=<INSTALL_DIR> --melang-dylib-prefix=<INSTALL_DIR> --debug
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1)

ExternalProject_Get_Property(melon INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

add_library(libmelon STATIC IMPORTED GLOBAL)
add_dependencies(libmelon melon)
set_target_properties(libmelon PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libmelon_static.a
    INTERFACE_LINK_DIRECTORIES ${INSTALL_DIR}/lib
    INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE libmelon)

