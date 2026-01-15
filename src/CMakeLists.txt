cmake_minimum_required(VERSION 3.31)
project(Selaura)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(FXC_EXE fxc.exe PATHS "C:/Program Files (x86)/Windows Kits/10/bin/*/x64")

set(SDK_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/scripts/sdk_parser.py)
set(PATTERNS_JSON ${CMAKE_CURRENT_SOURCE_DIR}/scripts/patterns.json)
set(SDK_DEF ${CMAKE_CURRENT_SOURCE_DIR}/sdk/sdk.def)
set(RESOLVER_HPP ${CMAKE_CURRENT_SOURCE_DIR}/memory/patterns/resolver.hpp)
set(SDK_LIB ${CMAKE_CURRENT_BINARY_DIR}/sdk.lib)

add_custom_command(
        OUTPUT ${SDK_DEF} ${RESOLVER_HPP}
        COMMAND Python3::Interpreter ${SDK_PARSER}
        DEPENDS ${SDK_PARSER} ${PATTERNS_JSON}
        VERBATIM
)

add_custom_command(
        OUTPUT ${SDK_LIB}
        COMMAND lib /nologo /def:${SDK_DEF} /out:${SDK_LIB} /machine:x64
        DEPENDS ${SDK_DEF}
        VERBATIM
)

add_custom_target(GenerateSdk DEPENDS ${SDK_DEF} ${SDK_LIB})

function(ADD_RESOURCES out_var)
    set(result)
    foreach(in_f ${ARGN})
        set(out_f "${CMAKE_CURRENT_BINARY_DIR}/${in_f}.o")
        get_filename_component(out_dir "${out_f}" DIRECTORY)
        file(MAKE_DIRECTORY "${out_dir}")
        add_custom_command(OUTPUT "${out_f}"
                COMMAND ld.exe -r -b binary -o "${out_f}" "${in_f}"
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${in_f}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                VERBATIM
        )
        list(APPEND result "${out_f}")
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE HLSL_SHADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "resources/shaders/*.hlsl")
set(SHADER_HEADERS "")
foreach(SHADER_FILE ${HLSL_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    if(SHADER_NAME MATCHES "\\.vs\\.hlsl$")
        set(SHADER_TYPE vs_5_0)
        string(REPLACE ".vs.hlsl" "_vs.h" HEADER_NAME ${SHADER_NAME})
    elseif(SHADER_NAME MATCHES "\\.ps\\.hlsl$")
        set(SHADER_TYPE ps_5_0)
        string(REPLACE ".ps.hlsl" "_ps.h" HEADER_NAME ${SHADER_NAME})
    else()
        continue()
    endif()
    set(HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/${HEADER_NAME}")
    get_filename_component(HEADER_DIR "${HEADER_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${HEADER_DIR}")
    string(MAKE_C_IDENTIFIER "g_${HEADER_NAME}" VAR_NAME)
    string(REPLACE "_h" "" VAR_NAME ${VAR_NAME})
    add_custom_command(
            OUTPUT "${HEADER_PATH}"
            COMMAND "${FXC_EXE}" /nologo /T ${SHADER_TYPE} /E main /Fh "${HEADER_PATH}" /Vn "${VAR_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}"
            VERBATIM
    )
    list(APPEND SHADER_HEADERS "${HEADER_PATH}")
endforeach()

CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:fmtlib/fmt#11.1.4")
CPMAddPackage("gh:skypjack/entt#fe8d7d78c4823e8a66a050bf86f5c6318cf76ce7")
CPMAddPackage("gh:cursey/SafetyHook#main")
CPMAddPackage("gh:ocornut/imgui#master")
CPMAddPackage("gh:g-truc/glm#master")
CPMAddPackage("gh:stephenberry/glaze#main")
CPMAddPackage("gh:nothings/stb#master")
CPMAddPackage("gh:DisabledMallis/magic_vtable#main")
CPMAddPackage(URI "gh:gabime/spdlog#v1.x" OPTIONS "SPDLOG_FMT_EXTERNAL ON")

set(MSDF_GEN_EXE "${CMAKE_SOURCE_DIR}/msdf-atlas-gen.exe")
file(GLOB_RECURSE TTF_FONTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "resources/*.ttf")

set(FONT_EMBEDDED_OBJS "")
foreach(FONT_PATH ${TTF_FONTS})
    get_filename_component(FONT_NAME ${FONT_PATH} NAME_WLE)
    set(RESOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")
    set(ATLAS_PNG "${RESOURCE_DIR}/${FONT_NAME}_msdf.png")
    set(ATLAS_JSON "${RESOURCE_DIR}/${FONT_NAME}_msdf.json")
    set(CHARSET_TXT "${RESOURCE_DIR}/${FONT_NAME}_charset.txt")
    set(OBJ_PNG "${ATLAS_PNG}.o")
    set(OBJ_JSON "${ATLAS_JSON}.o")
    file(MAKE_DIRECTORY "${RESOURCE_DIR}")
    if(FONT_NAME MATCHES "FontAwesome")
        set(CHARSET_CONTENT "[0x20, 0x7e], [0xf000, 0xf900]")
    else()
        set(CHARSET_CONTENT "[0x20, 0x7e]")
    endif()
    if(EXISTS "${CHARSET_TXT}")
        file(READ "${CHARSET_TXT}" OLD_CONTENT)
        if(NOT OLD_CONTENT STREQUAL CHARSET_CONTENT)
            file(WRITE "${CHARSET_TXT}" "${CHARSET_CONTENT}")
        endif()
    else()
        file(WRITE "${CHARSET_TXT}" "${CHARSET_CONTENT}")
    endif()
    add_custom_command(
            OUTPUT "${ATLAS_PNG}" "${ATLAS_JSON}"
            COMMAND "${MSDF_GEN_EXE}" -font "${CMAKE_CURRENT_SOURCE_DIR}/${FONT_PATH}"
            -format png -type msdf -size 128 -pxrange 8
            -imageout "${ATLAS_PNG}" -json "${ATLAS_JSON}"
            -charset "${CHARSET_TXT}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${FONT_PATH}" "${MSDF_GEN_EXE}" "${CHARSET_TXT}"
            VERBATIM
    )
    add_custom_command(
            OUTPUT "${OBJ_PNG}"
            COMMAND ld.exe -r -b binary -o "${OBJ_PNG}" "resources/${FONT_NAME}_msdf.png"
            DEPENDS "${ATLAS_PNG}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            VERBATIM
    )
    add_custom_command(
            OUTPUT "${OBJ_JSON}"
            COMMAND ld.exe -r -b binary -o "${OBJ_JSON}" "resources/${FONT_NAME}_msdf.json"
            DEPENDS "${ATLAS_JSON}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            VERBATIM
    )
    list(APPEND FONT_EMBEDDED_OBJS "${OBJ_PNG}" "${OBJ_JSON}")
endforeach()

file(GLOB_RECURSE RAW_RESOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} CONFIGURE_DEPENDS "resources/*")
if(RAW_RESOURCES)
    list(FILTER RAW_RESOURCES EXCLUDE REGEX ".*\\.ttf$")
    list(FILTER RAW_RESOURCES EXCLUDE REGEX ".*\\.hlsl$")
    add_resources(EMBEDDED_RESOURCES ${RAW_RESOURCES})
endif()
list(APPEND EMBEDDED_RESOURCES ${FONT_EMBEDDED_OBJS})

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
        core/renderer/sgfx.cpp
        core/renderer/renderer_d3d.cpp
        core/renderer/renderer_d3d11.cpp
        memory/hooks/CoreHooks.cpp
        memory/hooks/D3DHooks.cpp
        memory/hooks/InputHooks.cpp
        core/service/console.cpp
        core/service/notification_manager.cpp
        core/service/input_manager.cpp
        core/screen/screen.cpp
        core/screen/screens/clickgui.cpp
        ${EMBEDDED_RESOURCES}
        ${SHADER_HEADERS}
)

if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    target_compile_options(Selaura PRIVATE
            $<$<CONFIG:Release>:
            /O2 /Oi /Ot /Gw /Gy /fp:fast
            /EHs- /EHc- /GR- /D_HAS_EXCEPTIONS=0
            >
    )
    target_link_options(Selaura PRIVATE
            $<$<CONFIG:Release>:
            /OPT:REF /OPT:ICF /LTCG /INCREMENTAL:NO /MANIFEST:NO /FUNCTIONPADMIN
            >
            /DELAYLOAD:sdk.dll
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(Selaura PRIVATE $<$<CONFIG:Release>:-flto=thin>)
    target_link_options(Selaura PRIVATE $<$<CONFIG:Release>:-flto=thin>)
endif()

target_include_directories(Selaura PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders
        ${imgui_SOURCE_DIR}
        ${nlohmann_json_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
        ${magic_vtable_SOURCE_DIR}
)

target_link_libraries(Selaura PUBLIC
        libhat::libhat
        fmt::fmt
        EnTT
        safetyhook::safetyhook
        spdlog
        glm
        glaze::glaze
        ${SDK_LIB}
        Version.lib dwmapi.lib d3d11.lib d3d12.lib dxgi.lib d3dcompiler.lib gameinput.lib windowsapp
)

add_dependencies(Selaura GenerateSdk)
target_precompile_headers(Selaura PRIVATE pch.hpp)