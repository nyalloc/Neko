cmake_minimum_required(VERSION 3.15)

project(NekoSamples C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Modules")
include(ShaderCompiler)

function(set_neko_compiler_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()
endfunction()

add_subdirectory(ShaderConductor)
add_subdirectory(Neko)
add_subdirectory(SampleBase)
add_subdirectory(01_Triangle)
add_subdirectory(02_Cubes)
