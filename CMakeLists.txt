cmake_minimum_required(VERSION 3.20)

# Set project name
set(PROJECT_NAME libhdlc)
project(${PROJECT_NAME} C CXX)

# Set compiler flags
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Set compiler definitions
    add_definitions(-DHDLC_LOG_ENABLED)
endif()

# Include sub directories
add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(tests)