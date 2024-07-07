cmake_minimum_required(VERSION 3.10)

# Set the project name
project(search VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the C++ flags
if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
endif()

# Add the executable
add_executable(search search.cc)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties(search PROPERTIES
        OSX_ARCHITECTURES "x86_64;arm64"
    )
endif()
