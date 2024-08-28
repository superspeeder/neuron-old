cmake_minimum_required(VERSION 3.26)
project(neuron VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(Vulkan COMPONENTS shaderc_combined)

if (NOT ${Vulkan_FOUND})
    message(FATAL_ERROR "-- Vulkan not found")
endif()

if (Vulkan_shaderc_combined_FOUND)
    set(SHADERC_LIB Vulkan::shaderc_combined)
else()
    set(SHADERC_LIB shaderc_combined)
endif()

add_subdirectory(libs/)

add_library(neuron src/neuron/neuron.hpp
        src/neuron/neuron.cpp
        src/neuron/neuron.hpp
        src/neuron/os/window.cpp
        src/neuron/os/window.hpp
        src/neuron/graphics/gcontext.cpp
        src/neuron/graphics/gcontext.hpp
        src/neuron/math/utils.hpp
        src/neuron/math/utils.cpp
        src/neuron/utils/utils.cpp
        src/neuron/utils/utils.hpp)

target_include_directories(neuron PUBLIC src/)

target_link_libraries(neuron PUBLIC Vulkan::Vulkan ${SHADERC_LIB} glfw glm::glm spdlog::spdlog EnTT::EnTT stb::stb)
target_compile_definitions(neuron PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 NEURON_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} NEURON_VERSION_MINOR=${PROJECT_VERSION_MINOR} NEURON_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DGLFW_INCLUDE_NONE -DGLFW_INCLUDE_VULKAN)

add_library(neuron::neuron ALIAS neuron)

add_subdirectory(example/)
