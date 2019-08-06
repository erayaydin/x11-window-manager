cmake_minimum_required(VERSION 3.13)
project(wm)

set(CMAKE_CXX_STANDARD 11)

# SPDLOG
find_package(spdlog 1.3.1 REQUIRED)
add_library(spdlog INTERFACE IMPORTED)

add_executable(wm main.cpp WindowManager.cpp WindowManager.h)

target_link_libraries(wm spdlog::spdlog)
target_link_libraries(wm X11)