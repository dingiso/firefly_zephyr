cmake_minimum_required(VERSION 3.13.1)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
# set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}..)
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

find_package(Zephyr)

project(lock)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_sources(app PRIVATE main.cpp)
