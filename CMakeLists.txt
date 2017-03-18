cmake_minimum_required(VERSION 3.6)
project(project)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

set(SOURCE_FILES project/main.cpp project/Graph.cpp project/Graph.h project/SearchStructure.cpp project/SearchStructure.h project/GraphStack.cpp project/GraphStack.h project/GraphQueue.cpp project/GraphQueue.h)
add_executable(project ${SOURCE_FILES})