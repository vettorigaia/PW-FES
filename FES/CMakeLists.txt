cmake_minimum_required(VERSION 3.0.0)
project(stimulation_control VERSION 0.1 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

include_directories(
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/stimulation_libraries
)

add_executable(stimulation_control 
  "main.cpp" 
  "stimulation.cpp" 
  "stimulation.h"
  "tcp_socket.cpp"
)

add_library(libsmpt STATIC IMPORTED)
set_target_properties(libsmpt PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/stimulation_libraries/libsmpt.a)

target_link_libraries(stimulation_control 
  libsmpt pthread
)
