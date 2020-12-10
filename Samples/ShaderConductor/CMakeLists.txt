
set(SHADER_CONDUCTOR_INSTALL_DIR "SHADER_CONDUCTOR-NOTFOUND" CACHE PATH "Path to a prebuilt Shader Conductor")

# Create an import target for ShaderConductor so we can use the target name directly
add_executable(ShaderConductor IMPORTED GLOBAL)

if(CMAKE_HOST_WIN32)
	set(HOST_EXECUTABLE_SUFFIX ".exe" CACHE INTERNAL "")
elseif(CMAKE_HOST_APPLE)
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
elseif(CMAKE_HOST_UNIX)
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
endif()

set(ShaderConductor_BINARY_LOCATION "${SHADER_CONDUCTOR_INSTALL_DIR}/Bin/ShaderConductorCmd${HOST_EXECUTABLE_SUFFIX}")

set_target_properties(ShaderConductor PROPERTIES IMPORTED_LOCATION "${ShaderConductor_BINARY_LOCATION}")