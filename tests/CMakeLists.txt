# Set policy to avoid warning
cmake_policy(SET CMP0135 NEW)

# Include fetch content module
include(FetchContent)

# Include Google Test
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip)
FetchContent_MakeAvailable(googletest)

# Set executable name
set(EXE_NAME tests)

# Set source directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set source files
set(SRC_FILES ${SRC_DIR}/main.cpp)

# Create test executable
add_executable(${EXE_NAME} ${SRC_FILES})

# Link libraries
target_link_libraries(${EXE_NAME} PRIVATE hdlc gtest gtest_main)

# Set install directory
install(TARGETS ${EXE_NAME} DESTINATION tests)