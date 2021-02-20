cmake_minimum_required(VERSION 3.1)
project(VTabletServer C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
add_executable(VTabletServer main.c include/VTablet.h src/VTablet.c)