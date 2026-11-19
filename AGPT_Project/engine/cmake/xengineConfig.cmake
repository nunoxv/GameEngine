# Package config for the installed XEngine library:  find_package(xengine CONFIG REQUIRED)
include(CMakeFindDependencyMacro)
find_dependency(glm CONFIG)
find_dependency(SDL3 CONFIG)
find_dependency(box2d CONFIG)
find_dependency(glad CONFIG)

include("${CMAKE_CURRENT_LIST_DIR}/xengineTargets.cmake")

get_filename_component(_xe_prefix "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(XENGINE_SHADER_DIR "${_xe_prefix}/share/xengine/shaders")
include("${CMAKE_CURRENT_LIST_DIR}/xengineHelpers.cmake")
