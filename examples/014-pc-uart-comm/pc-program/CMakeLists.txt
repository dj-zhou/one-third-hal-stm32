cmake_minimum_required(VERSION 3.10)
project(pc-program)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(
  -Wall
  -Wextra
  -pedantic
  -Werror
  -Wformat=2
  -Wduplicated-cond
  -Wfloat-equal
  -Wshadow
  -Wconversion
  -Wlogical-not-parentheses
)

include_directories(./serial/)
include_directories(../comm/)

find_package(fmt)

# ------------------------------
add_executable(
  "pc-program"
  "src/main.cpp"
  "serial/serial.cpp"
  "../comm/comm.cpp"
)
target_link_libraries(
  "pc-program"
  fmt::fmt
)