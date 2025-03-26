# Set executable name
set(EXE_NAME basic)

# Set source directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set source files
set(SRC_FILES ${SRC_DIR}/main.c)

# Create executable
add_executable(${EXE_NAME} ${SRC_FILES})

# Link libraries
target_link_libraries(${EXE_NAME} PRIVATE hdlc)

# Set install directory
install(TARGETS ${EXE_NAME} DESTINATION examples)