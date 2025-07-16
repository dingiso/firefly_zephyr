cmake_minimum_required(VERSION 3.13.1)

#include(../common/basics.cmake)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(rothult_cordyceps)

target_sources(app PRIVATE main.cpp)
