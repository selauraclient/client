cmake_minimum_required(VERSION 3.31)
project(Selaura_Root)

include(cmake/CPM.cmake)
include(cmake/selaura_resource.cmake)

add_subdirectory(src)
add_subdirectory(loader)