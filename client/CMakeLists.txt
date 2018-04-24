cmake_minimum_required(VERSION 2.8)
project(client)
include_directories(includes ../shared/includes)
file(GLOB SOURCES "sources/*.cpp")
add_executable(client ${SOURCES})
target_link_libraries(client ${CMAKE_DL_LIBS} -lncurses -lpthread) # wat the?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

