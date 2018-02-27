cmake_minimum_required(VERSION 2.8)
project(MyProject)

#includowanie z RootProject
include_directories(${MyProject_SOURCE_DIR}/includes)

#okreslenie polozenia plikow source
set(PROJECT_SOURCE_DIR ${MyProject_SOURCE_DIR}/sources)

#zrobienie listy source'ow
set(MAIN_SRCS
${PROJECT_SOURCE_DIR}/main.cpp
)

#skladanie executable
add_executable(RootProject ${MAIN_SRCS})

#linkowanie z biblioteka dlLib
target_link_libraries(RootProject ${CMAKE_DL_LIBS}) #dlLib

#wlaczanie C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -O0 -g -std=c++11")

#instalowanie w folderze /exec/
install(TARGETS RootProject 
ARCHIVE DESTINATION ${MyProject_SOURCE_DIR}/exec/
LIBRARY DESTINATION ${MyProject_SOURCE_DIR}/exec/
RUNTIME DESTINATION ${MyProject_SOURCE_DIR}/exec/)

