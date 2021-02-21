cmake_minimum_required(VERSION 3.1)
project(OpenVTabServer C)

set(CMAKE_C_STANDARD 11)

include_directories(include /usr/include/libevdev-1.0)
add_executable(OpenVTabServer main.c include/OpenVTab.h src/OpenVTab.c)
target_link_libraries(OpenVTabServer evdev)