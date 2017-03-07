cmake_minimum_required(VERSION 3.6)
project(project)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Graph.cpp Graph.h SearchStructure.cpp SearchStructure.h GraphStack.cpp GraphStack.h GraphQueue.cpp GraphQueue.h)
add_executable(project ${SOURCE_FILES})