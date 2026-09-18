cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
project(HighwayTutor)
add_compile_options(-O2)
include_directories(include)
add_library(Highway SHARED
src/highway.cpp
)
add_executable(Simulator src/simulator.cpp)
target_link_libraries(Simulator Highway)

add_executable(Tutor src/tutor.cpp)
target_link_libraries(Tutor Highway)
