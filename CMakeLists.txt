cmake_minimum_required(VERSION 3.14)
project(sudoku-solver C)

set(CMAKE_C_STANDARD 90)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_compile_options(-fopenmp)

FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF()

add_executable(solver src/main.c src/node.h src/list.c src/list.h src/stack.c src/stack.h src/solver.c src/solver.h src/bulk_allocator.c src/bulk_allocator.h)
#target_sources(solver PUBLIC src/node_pointer.c)
target_sources(solver PUBLIC src/node_offset.c)
target_link_libraries(solver ${MPI_C_LIBRARIES})
target_link_libraries(solver m)

add_executable(generator src/sudoku_generator.c)