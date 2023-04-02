cmake_minimum_required(VERSION 3.13.1)

list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/../../pigweed
)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
# set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}..)
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

find_package(Zephyr)

project(lock)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

