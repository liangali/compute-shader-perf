cmake_minimum_required(VERSION 3.10)

project(ConvApp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE dxgi.lib d3d12.lib d3dcompiler.lib)