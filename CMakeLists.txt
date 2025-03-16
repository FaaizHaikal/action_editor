cmake_minimum_required(VERSION 3.10)
project(action_editor)

set(CMAKE_CXX_STANDARD 17)

# Include header files
include_directories(include)

# Add libraries
add_library(action STATIC src/action.cpp)
add_library(cmd_process STATIC src/cmd_process.cpp)

# Define the source files for the main executable
set(SRC_FILES src/main.cpp)  # Add other source files if needed

# Create the main executable
add_executable(main ${SRC_FILES})

# Link the libraries to the executable
target_link_libraries(main action cmd_process ncurses)