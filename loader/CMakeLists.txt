cmake_minimum_required(VERSION 3.31)
project(Selaura_Loader LANGUAGES CXX)

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

if (MSVC)
    target_compile_options(Selaura_Loader PRIVATE /GR-)
    target_compile_options(Selaura_Loader PRIVATE
        $<$<CONFIG:Release>:
        /O2
        /Ob2
        /Gy
        /Gw
        /DNDEBUG
        >
    )

    target_link_options(Selaura_Loader PRIVATE
        $<$<CONFIG:Release>:
        /LTCG
        /OPT:REF
        /OPT:ICF
        >
    )

    target_compile_options(Selaura_Loader PRIVATE
        $<$<CONFIG:Debug>:/Od>
    )
endif()
