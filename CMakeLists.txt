cmake_minimum_required(VERSION 3.10)

# Set the project name
project(search VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the C++ flags for GCC/Clang
if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
endif()

# Define build types
set(CMAKE_CONFIGURATION_TYPES Debug Release ASAN UBSAN TSAN MSAN COVERAGE RelWithDebInfo)

# Define ASAN build type flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_ASAN "/fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS_ASAN "/fsanitize=address")
else()
    set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS_ASAN "-fsanitize=address")
endif()

# Define UBSan build type flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_UBSAN "/fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS_UBSAN "/fsanitize=undefined")
else()
    set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all")
    set(CMAKE_EXE_LINKER_FLAGS_UBSAN "-fsanitize=undefined")
endif()

# Define TSan build type flags
if (MSVC)
    message(FATAL_ERROR "ThreadSanitizer is not supported by MSVC")
else()
    set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread")
    set(CMAKE_EXE_LINKER_FLAGS_TSAN "-fsanitize=thread")
endif()

# Define MSan build type flags
if (MSVC)
    message(FATAL_ERROR "MemorySanitizer is not supported by MSVC")
else()
    set(CMAKE_CXX_FLAGS_MSAN "-fsanitize=memory -fsanitize-memory-track-origins")
    set(CMAKE_EXE_LINKER_FLAGS_MSAN "-fsanitize=memory")
endif()

# Define Coverage build type flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_COVERAGE "/DEBUG /COVERAGE")
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "/DEBUG /COVERAGE")
else()
    set(CMAKE_CXX_FLAGS_COVERAGE "--coverage -O0 -g")
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
endif()

# Define RelWithDebInfo build type flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g")
endif()

# Add the executable
add_executable(search search.cc)

# Apply custom flags based on build type
function(add_custom_flags target)
    if (CMAKE_BUILD_TYPE STREQUAL "ASAN")
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    elseif (CMAKE_BUILD_TYPE STREQUAL "UBSAN")
        target_compile_options(${target} PRIVATE -fsanitize=undefined -fno-sanitize-recover=all)
        target_link_options(${target} PRIVATE -fsanitize=undefined)
    elseif (CMAKE_BUILD_TYPE STREQUAL "TSAN")
        target_compile_options(${target} PRIVATE -fsanitize=thread)
        target_link_options(${target} PRIVATE -fsanitize=thread)
    elseif (CMAKE_BUILD_TYPE STREQUAL "MSAN")
        target_compile_options(${target} PRIVATE -fsanitize=memory -fsanitize-memory-track-origins)
        target_link_options(${target} PRIVATE -fsanitize=memory)
    elseif (CMAKE_BUILD_TYPE STREQUAL "COVERAGE")
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        target_link_options(${target} PRIVATE --coverage)
    elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(${target} PRIVATE -O2 -g)
        target_link_options(${target} PRIVATE -O2 -g)
    endif()
endfunction()

# Apply custom flags to your executable
add_custom_flags(search)

# macOS specific settings
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties(search PROPERTIES
        OSX_ARCHITECTURES "x86_64;arm64"
    )
endif()
