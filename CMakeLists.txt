cmake_minimum_required(VERSION 3.10)
project(FeEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



add_executable(FeEngine
  src/main.cpp
  src/Engine.cpp
  src/Window.cpp
  src/VulkanContext.cpp
  src/Swapchain.cpp
  src/Timing.cpp
)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLFW REQUIRED glfw3)

target_compile_definitions(FeEngine PRIVATE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
)

target_include_directories(FeEngine PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

target_include_directories(FeEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(FeEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/include/engine/pch.hpp
)

target_link_libraries(FeEngine
    ${Vulkan_LIBRARIES}
    ${GLFW_LIBRARIES}
)

# ---- Shader compilation section ----

# Path to slang compiler (must be installed or built)
set(SLANGC_EXECUTABLE slangc)

# Input/output locations
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/src/shaders)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

# List of .slang shaders
set(SHADER_SOURCES
)

set(SHADER_BINARIES "")

foreach(SHADER ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
  set(OUTPUT_FILE ${SHADER_OUT_DIR}/${SHADER_NAME}.spv)

  add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${SLANGC_EXECUTABLE}
                ${SHADER}
                -target spirv
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name
                -o ${OUTPUT_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}.slang → SPIR-V"
        VERBATIM
    )

  list(APPEND SHADER_BINARIES ${OUTPUT_FILE})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SHADER_BINARIES})
add_dependencies(FeEngine compile_shaders)
