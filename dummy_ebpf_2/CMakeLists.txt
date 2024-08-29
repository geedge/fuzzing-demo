cmake_minimum_required(VERSION 3.10)

project(dummy_redirect)

find_package(PkgConfig)
pkg_check_modules(NL3 REQUIRED libnl-3.0 libnl-genl-3.0 libnl-route-3.0)

include_directories(include)

add_subdirectory(src)