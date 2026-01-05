cmake_minimum_required(VERSION 3.31)
project(Selaura_Loader)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Selaura_Loader
    main.cpp
)

target_link_libraries(Selaura_Loader
    PRIVATE
    shell32
)