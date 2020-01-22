cmake_minimum_required(VERSION 3.14)
project(LabNew)
find_package(nlohmann_json REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra")
add_executable(LabNew main.cpp Canvas.h RandomGenerator.cpp RandomGenerator.h)
target_link_libraries(LabNew nlohmann_json)
