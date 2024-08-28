add_subdirectory(entt)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(spdlog)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb/)
add_library(stb::stb ALIAS stb)
