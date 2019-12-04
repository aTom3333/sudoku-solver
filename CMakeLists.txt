cmake_minimum_required(VERSION 3.14)
project(sudoku-solver C)

set(CMAKE_C_STANDARD 90)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(solver src/main.c)
target_link_libraries(solver ${MPI_C_LIBRARIES})