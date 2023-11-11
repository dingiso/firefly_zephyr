cmake_minimum_required(VERSION 3.13.1)

# Only one board is supported
set(BOARD firefly_v1)

include(../common/basics.cmake)

project(activator)

add_subdirectory(../common common EXCLUDE_FROM_ALL)

target_sources(app PRIVATE main.cpp)
target_link_libraries(app PRIVATE
  cc1101
  color
  timer
  battery
  bluetooth
  rgb_led
)
