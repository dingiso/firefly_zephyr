cmake_minimum_required(VERSION 3.13.1)

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

include($ENV{PW_ROOT}/pw_build/pigweed.cmake)
include($ENV{PW_ROOT}/pw_protobuf_compiler/proto.cmake)

pw_proto_library(lock_test.test_proto
  SOURCES
    test.proto
  DEPS
    pw_protobuf.field_options_proto
)

pw_add_library(lock_test.system_server STATIC
  HEADERS
    system_server.h
  PRIVATE_DEPS
    pw_hdlc.pw_rpc
    pw_hdlc.rpc_channel_output
    pw_stream.sys_io_stream
    pw_log
  SOURCES
    system_server.cc
)

target_sources(app PRIVATE main.cpp)
target_link_libraries(app PRIVATE
  common.keyboard
  common.nfc
  pw_hdlc
  pw_hdlc.pw_rpc
  pw_rpc.server
  lock_test.system_server
  lock_test.test_proto.pwpb
  lock_test.test_proto.pwpb_rpc
  pw_unit_test.light
  printk_event_handler
  pw_log.protos.raw_rpc
  pw_log
)

