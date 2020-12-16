
function(add_resource_Library name)

endfunction()

#[[
    add_shader_resource

    This function will invoke ShaderConductor and generate C++ code with the compiled shader
    IR embedded inside it. Some CMake crimes will be committed to ensure that the resulting
    code is recompiled when the shader sources are updated. In theory, this will let every
    example / backend share the same HLSL shaders. I don't expect this to work perfectly,
    but I'll figure out the problems along the way and learn a lot as I try and fix them.

    ShaderConductor seems to only support shader modules with 1 entry point (not sure why yet).
]]
function(add_shader_resource name)
    get_filename_component(ShaderBin "${CMAKE_CURRENT_BINARY_DIR}/${name}ShaderBin" ABSOLUTE)
    message(STATUS "Writing shader binaries to ${ShaderBin}")

    set(options)
    set(one_value_args NAME INPUT ENTRY_POINT STAGE)
    set(multi_value_args)
    cmake_parse_arguments(
        SHADER
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    message(STATUS "\nShader sources:\n\t${SHADER_NAME}, ${SHADER_INPUT}, ${SHADER_ENTRY_POINT}, ${SHADER_STAGE}\n")
endfunction()
