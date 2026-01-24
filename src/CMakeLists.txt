cmake_minimum_required(VERSION 3.31)
project(Selaura)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

set(CPM_DOWNLOAD_ALL TRUE)
set(CPM_USE_LOCAL_PACKAGES TRUE)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN_DIR})
set(SDK_DEF ${GEN_DIR}/sdk.def)
set(RESOLVER_HPP ${GEN_DIR}/resolver.hpp)
set(SDK_LIB ${GEN_DIR}/sdk.lib)

add_custom_command(
    OUTPUT ${SDK_DEF} ${RESOLVER_HPP}
    COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/scripts/sdk_parser.py"
    --input "${CMAKE_CURRENT_SOURCE_DIR}/scripts/patterns.json"
    --def-out ${SDK_DEF} --cpp-out ${RESOLVER_HPP}
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/sdk_parser.py" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/patterns.json"
    VERBATIM
)

add_custom_command(
    OUTPUT ${SDK_LIB}
    COMMAND lib /nologo /def:${SDK_DEF} /out:${SDK_LIB} /machine:x64
    DEPENDS ${SDK_DEF}
    VERBATIM
)
add_custom_target(GenerateSdk DEPENDS ${SDK_DEF} ${SDK_LIB})

selaura_compile_shaders(SHADER_HEADERS)
selaura_process_fonts(FONT_EMBEDDED_OBJS)

file(GLOB_RECURSE RAW_RESOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} CONFIGURE_DEPENDS "resources/*")
if(RAW_RESOURCES)
    list(FILTER RAW_RESOURCES EXCLUDE REGEX ".*\\.ttf$")
    list(FILTER RAW_RESOURCES EXCLUDE REGEX ".*\\.hlsl$")
    selaura_add_resources(EMBEDDED_RESOURCES ${RAW_RESOURCES})
endif()
list(APPEND EMBEDDED_RESOURCES ${FONT_EMBEDDED_OBJS})

CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:fmtlib/fmt#11.1.4")
CPMAddPackage("gh:skypjack/entt#fe8d7d78c4823e8a66a050bf86f5c6318cf76ce7")
CPMAddPackage("gh:cursey/SafetyHook#983ba5c4b72866c8ed5020b6d57b7108fd1622c2")
CPMAddPackage("gh:g-truc/glm#e7970a8b26732f1b0df9690f7180546f8c30e48e")
CPMAddPackage("gh:stephenberry/glaze@7.0.1")
CPMAddPackage("gh:nothings/stb#f1c79c02822848a9bed4315b12c8c8f3761e1296")
CPMAddPackage("gh:DisabledMallis/magic_vtable#dbc41e2d13766a644ea88cbd4b839202e01ba582")
CPMAddPackage("gh:microsoft/GSL#756c91ab895aa52f650599bb1a3fc131f1f4b5ef")
CPMAddPackage(URI "gh:gabime/spdlog@1.17.0" OPTIONS "SPDLOG_FMT_EXTERNAL ON")

file(GLOB_RECURSE SELAURA_SOURCES CONFIGURE_DEPENDS "core/*.cpp" "memory/*.cpp" "sdk/*.cpp")

add_library(Selaura SHARED
        ${SELAURA_SOURCES}
        ${EMBEDDED_RESOURCES}
        ${SHADER_HEADERS}
        ${RESOLVER_HPP}
)

if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    target_compile_options(Selaura PRIVATE $<$<CONFIG:Release>:/O2 /Oi /Ot /Gw /Gy /fp:fast /EHs- /EHc- /GR- /D_HAS_EXCEPTIONS=0>)
    target_link_options(Selaura PRIVATE $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /LTCG /INCREMENTAL:NO /MANIFEST:NO> /DELAYLOAD:sdk.dll)
endif()

target_include_directories(Selaura PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_DIR} ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders
        ${stb_SOURCE_DIR} ${magic_vtable_SOURCE_DIR}
)

target_link_libraries(Selaura PUBLIC
        libhat::libhat
        fmt::fmt
        EnTT
        safetyhook::safetyhook
        spdlog
        glm
        glaze::glaze
        Microsoft.GSL::GSL
        ${SDK_LIB}
        Version.lib
        dwmapi.lib
        d3d11.lib
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
        gameinput.lib
        windowsapp
        Shlwapi.lib
        gdiplus.lib
)

add_dependencies(Selaura GenerateSdk)
target_precompile_headers(Selaura PRIVATE pch.hpp)