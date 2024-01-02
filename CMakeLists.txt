cmake_minimum_required(VERSION 3.12)
project(crud-cpp)

add_executable(${PROJECT_NAME} crud.cpp main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
