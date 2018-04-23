cmake_minimum_required(VERSION 2.8)
project(server)
include_directories(includes ../shared/includes)
file(GLOB SOURCES "sources/*.cpp")
add_executable(server ${SOURCES})
target_link_libraries(server ${CMAKE_DL_LIBS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

