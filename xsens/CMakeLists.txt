cmake_minimum_required(VERSION 3.0.2)
project(mtw_driver VERSION 0.3.0)


# Set binary output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_COMPILER "/usr/bin/g++-7")
set(CMAKE_C_COMPILER "/usr/bin/gcc-7")



find_program(AWK awk)
message(STATUS "Found awk: ${AWK}")
# Define MTW_VERSION with 'git describe' + 'git branch'
execute_process(
  COMMAND /bin/sh -c "echo $(git describe)-$(git status | \"${AWK}\" 'NR==1{print$3}')"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE MTW_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "version: ${MTW_VERSION}")

# C++ Flags
set(CMAKE_CXX_FLAGS "-Wno-deprecated")
# add_compile_options(-std=c++14)

# System dependencies
find_package(Boost REQUIRED COMPONENTS system)

# Header files (.h files)
include_directories(
  ./include/
  /usr/local/xsens/public/include/
  /usr/local/xsens/include/
)

# Libraries
link_directories(
  /usr/local/xsens/lib64/
)

# Target mtw_driver_imu_dev
add_executable(${PROJECT_NAME}_imu
  mtw_imu_main.cpp
  src/conio.c
  src/console.cpp
  src/findClosestUpdateRate.cpp
  src/myxda.cpp
  src/pstream_gettid.cpp
)

# Add version as define macro for the target "imu"
target_compile_definitions(${PROJECT_NAME}_imu
  PRIVATE "MTW_VERSION=\"${MTW_VERSION}\""
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}_imu
  xsensdeviceapi
  xstypes
  pthread
  dl
)