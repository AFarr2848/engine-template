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
  src/managers/BufferManager.cpp
  src/managers/ShaderManager.cpp
  src/World.cpp
)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(assimp REQUIRED)

pkg_check_modules(GLFW REQUIRED glfw3)

target_compile_definitions(FeEngine PRIVATE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
)

target_compile_definitions(FeEngine PRIVATE
VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
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
    assimp
)

set(SLANGC_EXECUTABLE slangc)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/src/shaders)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SHADER_SOURCES
    ${SHADER_DIR}/triangle.slang
)

set(SHADER_BINARIES "")

foreach(SHADER ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER} NAME_WE)

  set(VERT_OUTPUT ${SHADER_OUT_DIR}/${SHADER_NAME}_vert.spv)
  set(FRAG_OUTPUT ${SHADER_OUT_DIR}/${SHADER_NAME}_frag.spv)

  add_custom_command(
        OUTPUT ${VERT_OUTPUT}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER}
                -entry vertexMain
                -stage vertex
                -target spirv
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name
                -o ${VERT_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling Vrt ${SHADER_NAME}.slang → ${SHADER_NAME}_vert.spv"
        VERBATIM
    )

  add_custom_command(
        OUTPUT ${FRAG_OUTPUT}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER}
                -entry fragmentMain
                -stage fragment
                -target spirv
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name
                -o ${FRAG_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling Frg ${SHADER_NAME}.slang → ${SHADER_NAME}_frag.spv"
        VERBATIM
    )

  list(APPEND SHADER_BINARIES ${VERT_OUTPUT} ${FRAG_OUTPUT})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SHADER_BINARIES})
add_dependencies(FeEngine compile_shaders)
