cmake_minimum_required(VERSION 3.13.1)

# Pigweed used 16k by default which is A LOT.
add_definitions(-DPW_UNIT_TEST_CONFIG_MEMORY_POOL_SIZE=512)

list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/../../pigweed
)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
# set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}..)
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

find_package(Zephyr)

project(lock_test)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(../testing testing)

target_sources(app PRIVATE main.cpp)
target_link_libraries(app PRIVATE
  pw_unit_test.light
  printk_event_handler
)
