cmake_minimum_required(VERSION 3.13.1)

include(../common/basics.cmake)

project(lock)

add_subdirectory(../common common EXCLUDE_FROM_ALL)

target_sources(app PRIVATE main.cpp)
target_link_libraries(app PRIVATE
  pw_log
  common.keyboard
  common.nfc
  bluetooth
  buzzer
  rgb_led
)

