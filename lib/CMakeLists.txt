# Set library name
set(LIB_NAME hdlc)

# Set source directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set include directory
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set source files
set(SRC_FILES ${SRC_DIR}/hdlc.c)

# Create library
add_library(${LIB_NAME} STATIC ${SRC_FILES})

# Set include directories
target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

# Set install directory
install(TARGETS ${LIB_NAME} DESTINATION lib)