cmake_minimum_required(VERSION 3.19.0)
project(MPChess VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3 -Og -Wall -Wpedantic -Wextra -fconcepts-diagnostics-depth=3 -latomic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native -latomic")

add_subdirectory(${PROJECT_SOURCE_DIR}/src)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()
