
add_library(SampleBase)
target_include_directories(SampleBase PUBLIC "Include" PRIVATE "Source")
target_compile_definitions(SampleBase PRIVATE SDL_MAIN_HANDLED)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(SampleBase PRIVATE SDL2::SDL2 SDL2::SDL2main)

target_sources(SampleBase PRIVATE "Source/SampleBase.c")
target_link_libraries(SampleBase PUBLIC Neko)
