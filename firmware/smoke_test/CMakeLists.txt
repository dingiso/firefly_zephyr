cmake_minimum_required(VERSION 3.13.1)

include(../common/basics.cmake)

project(smoke_test)

add_subdirectory(../common common EXCLUDE_FROM_ALL)
add_subdirectory(../testing testing EXCLUDE_FROM_ALL)

target_sources(app PRIVATE main.cpp)
target_link_libraries(app PRIVATE
  color
  timer
  cc1101
  rgb_led
  buzzer
  pw_unit_test.light
  printk_event_handler
)
