
add_library(Neko STATIC "Source/Neko.c")
target_include_directories(Neko PUBLIC ../../Include)

find_package(Vulkan REQUIRED)
target_link_libraries(Neko PRIVATE Vulkan::Vulkan)
target_compile_definitions(Neko PUBLIC NK_VULKAN_IMPLEMENTATION)
