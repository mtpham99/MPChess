find_package(Catch2 2.13.8...<3.0.0 REQUIRED)

# Not all files are needed for perft
# TODO : remove un-needed source (and remove atomic from link)
file(GLOB
     MPChess_SRC
     ${PROJECT_SOURCE_DIR}/src/*.cpp
)
list(REMOVE_ITEM MPChess_SRC ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_executable(perft_tests perft_tests.cpp ${MPChess_SRC})
target_include_directories(perft_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(perft_tests PRIVATE Catch2::Catch2 atomic)
set_target_properties(perft_tests
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

include(CTest)
include(Catch)
catch_discover_tests(perft_tests)
