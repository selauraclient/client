cmake_minimum_required(VERSION 3.31)

function(selaura_write_if_changed path content)
    set(update_required OFF)
    if(EXISTS "${path}")
        file(READ "${path}" current_content)
        if(NOT "${current_content}" STREQUAL "${content}")
            set(update_required ON)
        endif()
    else()
        set(update_required ON)
    endif()

    if(update_required)
        file(WRITE "${path}" "${content}")
    endif()
endfunction()

function(selaura_add_resources out_var)
    set(result)
    foreach(in_f ${ARGN})
        set(out_f "${CMAKE_CURRENT_BINARY_DIR}/${in_f}.o")
        get_filename_component(out_dir "${out_f}" DIRECTORY)
        file(MAKE_DIRECTORY "${out_dir}")
        add_custom_command(
                OUTPUT "${out_f}"
                COMMAND ld.exe -r -b binary -o "${out_f}" "${in_f}"
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${in_f}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                VERBATIM
        )
        list(APPEND result "${out_f}")
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

function(selaura_compile_shaders out_var)
    file(GLOB_RECURSE hlsl_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "resources/shaders/*.hlsl")
    set(headers "")
    foreach(f ${hlsl_files})
        get_filename_component(name ${f} NAME)
        if(name MATCHES "\\.vs\\.hlsl$")
            set(type vs_5_0)
            string(REPLACE ".vs.hlsl" "_vs.h" header_name ${name})
        elseif(name MATCHES "\\.ps\\.hlsl$")
            set(type ps_5_0)
            string(REPLACE ".ps.hlsl" "_ps.h" header_name ${name})
        else()
            continue()
        endif()

        set(h_path "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/${header_name}")
        get_filename_component(h_dir "${h_path}" DIRECTORY)
        file(MAKE_DIRECTORY "${h_dir}")

        string(MAKE_C_IDENTIFIER "g_${header_name}" var_name)
        string(REPLACE "_h" "" var_name ${var_name})

        add_custom_command(
                OUTPUT "${h_path}"
                COMMAND "fxc.exe" /nologo /T ${type} /E main /Fh "${h_path}" /Vn "${var_name}" "${CMAKE_CURRENT_SOURCE_DIR}/${f}"
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${f}"
                VERBATIM
        )
        list(APPEND headers "${h_path}")
    endforeach()
    set(${out_var} "${headers}" PARENT_SCOPE)
endfunction()

function(selaura_process_fonts out_var)
    set(msdf_exe "${CMAKE_SOURCE_DIR}/msdf-atlas-gen.exe")
    file(GLOB_RECURSE ttf_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "resources/*.ttf")
    set(objs "")

    foreach(f ${ttf_files})
        get_filename_component(f_name ${f} NAME_WLE)
        set(res_dir "${CMAKE_CURRENT_BINARY_DIR}/resources")
        file(MAKE_DIRECTORY "${res_dir}")

        set(png "${res_dir}/${f_name}_msdf.png")
        set(json "${res_dir}/${f_name}_msdf.json")
        set(charset_txt "${res_dir}/${f_name}_charset.txt")

        set(charset_ascii "[0x20, 0x7e]")
        set(charset_icons "[0xf000, 0xf900]")

        if(f_name MATCHES "FontAwesome")
            set(content "${charset_ascii}, ${charset_icons}")
        else()
            set(content "${charset_ascii}")
        endif()

        selaura_write_if_changed("${charset_txt}" "${content}")

        add_custom_command(
                OUTPUT "${png}" "${json}"
                COMMAND "${msdf_exe}" -font "${CMAKE_CURRENT_SOURCE_DIR}/${f}"
                -format png -type msdf -size 128 -pxrange 8
                -imageout "${png}" -json "${json}" -charset "${charset_txt}"
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${f}" "${msdf_exe}" "${charset_txt}"
                VERBATIM
        )

        foreach(ext png json)
            set(obj "${res_dir}/${f_name}_msdf.${ext}.o")
            add_custom_command(
                    OUTPUT "${obj}"
                    COMMAND ld.exe -r -b binary -o "${obj}" "resources/${f_name}_msdf.${ext}"
                    DEPENDS "${res_dir}/${f_name}_msdf.${ext}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    VERBATIM
            )
            list(APPEND objs "${obj}")
        endforeach()
    endforeach()
    set(${out_var} "${objs}" PARENT_SCOPE)
endfunction()