configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/mc_panda/config.h")
configure_file(api.in.h "${CMAKE_CURRENT_BINARY_DIR}/include/mc_panda/devices/api.h")

# Panda custom devices library
set(devices_HDR
  devices/Robot.h
  devices/Pump.h
  "${CMAKE_CURRENT_BINARY_DIR}/include/mc_panda/devices/api.h"
)
set(devices_SRC
  devices/Robot.cpp
  devices/Pump.cpp
)

if(NOT MC_RTC_BUILD_STATIC)
  add_library(mc_panda_devices SHARED ${devices_SRC} ${devices_HDR})
  add_library(mc_panda::devices ALIAS mc_panda_devices)
  set_target_properties(mc_panda_devices PROPERTIES EXPORT_NAME devices)
  target_include_directories(mc_panda_devices PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>$<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
  target_link_libraries(mc_panda_devices PUBLIC mc_rtc::mc_rbdyn)
  if(MC_PANDA_WITH_FRANKA)
    target_link_libraries(mc_panda_devices PUBLIC Franka::Franka)
  else()
    target_compile_definitions(mc_panda_devices PUBLIC MC_PANDA_WITHOUT_FRANKA)
  endif()
  target_compile_definitions(mc_panda_devices PRIVATE -DMC_PANDA_DEVICES_EXPORTS)
  install(TARGETS mc_panda_devices
    EXPORT mc_pandaTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
else()
  target_sources(mc_rbdyn PRIVATE ${devices_SRC} ${devices_HDR})
  if(MC_PANDA_WITH_FRANKA)
    target_link_libraries(mc_rbdyn PUBLIC Franka::Franka)
  else()
    target_compile_definitions(mc_rbdyn PUBLIC MC_PANDA_WITHOUT_FRANKA)
  endif()
  target_include_directories(mc_rbdyn PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>$<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
  target_compile_definitions(mc_rbdyn PRIVATE -DMC_PANDA_DEVICES_EXPORTS)
  add_library(mc_panda::devices ALIAS mc_rbdyn)
endif()
install(FILES ${devices_HDR} DESTINATION include/mc_panda/devices)

# Panda robot module library
add_robot(panda panda.cpp panda.h)
if(MC_RTC_BUILD_STATIC)
  target_link_libraries(mc_rbdyn PRIVATE mc_panda::devices)
  target_include_directories(mc_rbdyn PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
else()
  target_link_libraries(panda PRIVATE mc_panda::devices)
  target_include_directories(panda PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
endif()

# Panda custom states library
add_subdirectory(states)

if(NOT MC_RTC_BUILD_STATIC)
  install(EXPORT mc_pandaTargets
    FILE mc_pandaTargets.cmake
    NAMESPACE mc_panda::
    DESTINATION lib/cmake/mc_panda)
endif()
