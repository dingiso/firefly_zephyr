cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

project(firefly)

target_sources(app PRIVATE
  src/cc1101.cpp
  src/main.cpp
  src/rgb_led.cpp
)
target_include_directories(app PRIVATE src)
