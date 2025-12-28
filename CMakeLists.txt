cmake_minimum_required(VERSION 3.31)
project(Selaura)

set(CMAKE_CXX_STANDARD 23)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

set(SDK_PARSER
        ${CMAKE_CURRENT_SOURCE_DIR}/memory/delayloader/sdk_parser.py
)

set(PATTERNS_JSON
        ${CMAKE_CURRENT_SOURCE_DIR}/memory/patterns/patterns.json
)

set(SDK_DEF
        ${CMAKE_CURRENT_SOURCE_DIR}/sdk/sdk.def
)

set(RESOLVER_HPP
        ${CMAKE_CURRENT_SOURCE_DIR}/memory/patterns/resolver.hpp
)

set(SDK_LIB
        ${CMAKE_CURRENT_BINARY_DIR}/sdk.lib
)

add_custom_command(
        OUTPUT
        ${SDK_DEF}
        ${RESOLVER_HPP}
        COMMAND
        Python3::Interpreter ${SDK_PARSER}
        DEPENDS
        ${SDK_PARSER}
        ${PATTERNS_JSON}
        COMMENT "Generating sdk.def and resolver.hpp"
        VERBATIM
)

add_custom_command(
        OUTPUT ${SDK_LIB}
        COMMAND lib /nologo /def:${SDK_DEF} /out:${SDK_LIB} /machine:x64
        DEPENDS ${SDK_DEF}
        COMMENT "Generating import library ${SDK_LIB} from ${SDK_DEF}"
        VERBATIM
)

add_custom_target(GenerateSdk
        DEPENDS
        ${SDK_DEF}
        ${SDK_LIB}
)

include(cmake/CPM.cmake)

CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:fmtlib/fmt#11.1.4")
CPMAddPackage("gh:skypjack/entt#fe8d7d78c4823e8a66a050bf86f5c6318cf76ce7")
CPMAddPackage("gh:cursey/SafetyHook#main")
CPMAddPackage("gh:ocornut/imgui#master")
CPMAddPackage(
        URI "gh:gabime/spdlog#v1.x"
        OPTIONS
            "SPDLOG_FMT_EXTERNAL ON"
)

add_library(Selaura SHARED
        core/dllmain.cpp

        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp

        memory/hooks/D3DHooks.cpp
        memory/hooks/InputHooks.cpp

        core/console.cpp
)

target_include_directories(Selaura PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
)

target_link_libraries(Selaura PUBLIC
        libhat::libhat
        fmt::fmt
        EnTT
        safetyhook::safetyhook
        spdlog
        ${SDK_LIB}
        Version.lib
        dwmapi.lib
        d3d11.lib
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
        gameinput.lib
)

add_dependencies(Selaura GenerateSdk)

target_link_options(Selaura PRIVATE "/DELAYLOAD:sdk.dll")
target_precompile_headers(Selaura PRIVATE pch.hpp)
