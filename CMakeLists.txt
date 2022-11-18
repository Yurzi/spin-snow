cmake_minimum_required(VERSION 3.23)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "" FORCE)
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/ports")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# [project]
project(spin-snow
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "a spin-snow"
    )


set(CMAKE_BUILD_TYPE "Debug")

# [compiler]
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# [source]
set(SRC_LIST
  src/utils.cc
  src/main.cc
  src/shader.cc
  src/mesh.cc
  src/model.cc
  src/stb_image.cc
  )

# [dependencies]
find_path(STB_INCLUDE_DIRS "stb_image.h")
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/src)

# [library]
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad glfw glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp imgui::imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${STB_INCLUDE_DIRS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
