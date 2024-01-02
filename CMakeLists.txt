cmake_minimum_required(VERSION 3.12)
project(crud-cpp)

add_executable(${PROJECT_NAME} crud.cpp main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
