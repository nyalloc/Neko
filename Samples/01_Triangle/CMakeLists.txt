
add_executable(01_Triangle Triangle.c Triangle.hlsl)
target_link_libraries(01_Triangle PUBLIC SampleBase)

set_source_files_properties(Triangle.hlsl PROPERTIES VS_TOOL_OVERRIDE "None")
