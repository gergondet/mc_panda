cmake_minimum_required(VERSION 3.1)

# Disable in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "This project doesn't support in-source builds")
endif()

project(mc_panda LANGUAGES CXX VERSION 1.0.0)
enable_testing()
set(CMAKE_CXX_STANDARD 11)

option(MC_PANDA_WITH_FRANKA "Build mc_panda with libfranka integration" ON)

if(NOT TARGET mc_rtc::mc_control)
  find_package(mc_rtc 1.4.0 REQUIRED)
endif()
if(MC_PANDA_WITH_FRANKA)
  find_package(Franka REQUIRED)
endif()

add_subdirectory(cmake)
add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(tests)
