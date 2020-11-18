cmake_minimum_required(VERSION 3.1)

# Disable in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "This project doesn't support in-source builds")
endif()

project(mc_panda LANGUAGES CXX VERSION 1.0.0)
enable_testing()
set(CMAKE_CXX_STANDARD 11)

option(MC_PANDA_WITH_FRANKA "Build mc_panda with libfranka integration" ON)
option(MC_PANDA_DISABLE_XACRO "Force mc_panda not to use xacro" OFF)

if(MC_PANDA_WITH_FRANKA)
  find_package(Franka REQUIRED)
endif()

if(NOT TARGET mc_rtc::mc_control)
  find_package(mc_rtc 1.4.0 REQUIRED)
else()
  if(MC_RTC_BUILD_STATIC)
    # We set properties on mc_rbdyn and mc_control_fsm, this requires CMake 3.13
    # See https://gitlab.kitware.com/cmake/cmake/-/merge_requests/2370
    cmake_minimum_required(VERSION 3.13)
    if(MC_PANDA_WITH_FRANKA)
      add_project_dependency(Franka REQUIRED)
    endif()
  endif()
endif()
add_subdirectory(cmake)
add_subdirectory(data)
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
