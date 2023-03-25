cmake_minimum_required(VERSION 3.13.1)

# We create a pw::Function capturing 3 4-byte argument.
add_definitions(-DPW_FUNCTION_INLINE_CALLABLE_SIZE=12)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
endif()

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

add_subdirectory(../testing testing EXCLUDE_FROM_ALL)
add_subdirectory(../common common EXCLUDE_FROM_ALL)
add_subdirectory(../common/rpc common_rpc EXCLUDE_FROM_ALL)

include($ENV{PW_ROOT}/pw_build/pigweed.cmake)
include($ENV{PW_ROOT}/pw_protobuf_compiler/proto.cmake)

target_sources(app PRIVATE main.cpp)
target_link_libraries(app PRIVATE
  pw_hdlc
  pw_hdlc.pw_rpc
  pw_rpc.server
  rpc.system_server
  rpc.test_proto.pwpb
  rpc.test_proto.pwpb_rpc
  pw_unit_test.light
  printk_event_handler
  pw_log.protos.raw_rpc
  pw_log
)

