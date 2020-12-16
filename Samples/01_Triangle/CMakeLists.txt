
add_executable(01_Triangle Triangle.c)

add_resource_library(TriangleShaders)

add_shader_resource(TriangleShaders
    NAME TriangleVertex
    INPUT Triangle.hlsl
    ENTRY_POINT vsMain
    STAGE vertex
)

add_shader_resource(TriangleShaders
    NAME TrianglePixel
    INPUT Triangle.hlsl
    ENTRY_POINT psMain
    STAGE pixel
)

target_link_libraries(01_Triangle PUBLIC
    SampleBase
    TriangleShaders
)
