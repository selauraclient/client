cmake_minimum_required(VERSION 3.31)
project(Selaura)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
add_library(Selaura SHARED
    dllmain.cpp
    platform/win32.cpp
    sdk/sdk.cpp
    sdk/import/basic.cpp
    sdk/import/iat_write.cpp
    sdk/import/img_delay_description.cpp
    sdk/import/importer.cpp
)

include(cmake/CPM.cmake)

CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:fmtlib/fmt#11.1.4")
CPMAddPackage("gh:skypjack/entt#fe8d7d78c4823e8a66a050bf86f5c6318cf76ce7")

target_include_directories(Selaura PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SDK_DEF "${CMAKE_CURRENT_SOURCE_DIR}/sdk/sdk.def")
set(SDK_LIB "${CMAKE_CURRENT_BINARY_DIR}/sdk.lib")

add_custom_command(
    OUTPUT ${SDK_LIB}
    COMMAND lib /nologo /def:${SDK_DEF} /out:${SDK_LIB} /machine:x64
    DEPENDS ${SDK_DEF}
    COMMENT "Generating import library ${SDK_LIB} from ${SDK_DEF}"
    VERBATIM
)

target_link_libraries(Selaura PUBLIC
    libhat::libhat
    fmt::fmt
    EnTT
    ${CMAKE_CURRENT_BINARY_DIR}/sdk.lib
    Version.lib
    dwmapi.lib
)

target_link_options(Selaura PRIVATE "/DELAYLOAD:sdk.dll")
target_precompile_headers(Selaura PRIVATE pch.hpp)