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

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

project(smoke_test)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(../common common)

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

# Remove when migrating to the Zephyr RTOS version containing
# https://github.com/zephyrproject-rtos/zephyr/commit/82caf98dde455f9a7a9b97a6c181072ab626e5c0
target_compile_options(pw_sys_io_zephyr PRIVATE -Wno-error=missing-field-initializers)
