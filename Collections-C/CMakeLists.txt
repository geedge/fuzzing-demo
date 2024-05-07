cmake_minimum_required(VERSION 3.0)

project(collectionc_test)

include(ExternalProject)

##### collection-c
ExternalProject_Add(collectionc PREFIX collectionc
    URL ${CMAKE_CURRENT_SOURCE_DIR}/collectionc-6b6ec21.zip
    URL_MD5 965a56ff42165e57054b50139c55b45a
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSHARED=False)

ExternalProject_Get_Property(collectionc SOURCE_DIR)
ExternalProject_Get_Property(collectionc INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

message(STATUS "SOURCE_DIR: ${SOURCE_DIR}")

add_library(libcollectionc STATIC IMPORTED GLOBAL)
add_dependencies(libcollectionc collectionc)
set_target_properties(libcollectionc PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcollectc.a
    INTERFACE_LINK_DIRECTORIES ${INSTALL_DIR}/lib
    INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR}/src/include
)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE libcollectionc)

