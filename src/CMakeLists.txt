file(GLOB
     MPChess_SRC
     ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(main main.cpp ${MPChess_SRC})
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(main PRIVATE atomic)
set_target_properties(main 
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)
