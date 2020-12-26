
add_executable(01_Triangle Triangle.c)

add_shader_module(01_Triangle
    NAME TriangleVertex
    INPUT Triangle.hlsl
    ENTRY_POINT vs_main
    STAGE vs
)

add_shader_module(01_Triangle
    NAME TriangleFragment
    INPUT Triangle.hlsl
    ENTRY_POINT ps_main
    STAGE ps
)

target_link_libraries(01_Triangle PUBLIC
    SampleBase
)

set_neko_compiler_options(01_Triangle)
