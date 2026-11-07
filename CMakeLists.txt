cmake_minimum_required(VERSION 3.13)

# Set the project name
project(search VERSION 1.0)

# Default to an optimized build on single-config generators
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the C++ flags for GCC/Clang
if (MSVC)
    message(FATAL_ERROR "MSVC is not supported")
endif()
add_compile_options(-Wall -Wextra -pedantic)

# Define build types
set(CMAKE_CONFIGURATION_TYPES Debug Release ASAN UBSAN TSAN MSAN COVERAGE RelWithDebInfo)

# MemorySanitizer isn't available with Apple clang
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MSAN)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
    if (BUILD_TYPE_UPPER STREQUAL "MSAN")
        message(FATAL_ERROR "MSAN is not supported on macOS")
    endif()
endif()

# Define ASAN build type flags
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_ASAN "-fsanitize=address")

# Define UBSan build type flags
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all")
set(CMAKE_EXE_LINKER_FLAGS_UBSAN "-fsanitize=undefined")

# Define TSan build type flags
set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread")
set(CMAKE_EXE_LINKER_FLAGS_TSAN "-fsanitize=thread")

# Define MSan build type flags
set(CMAKE_CXX_FLAGS_MSAN "-fsanitize=memory -fsanitize-memory-track-origins")
set(CMAKE_EXE_LINKER_FLAGS_MSAN "-fsanitize=memory")

# Define Coverage build type flags
set(CMAKE_CXX_FLAGS_COVERAGE "--coverage -O0 -g")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")

# Define RelWithDebInfo build type flags
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g")

# Add the executable
add_executable(search search.cc)

# macOS specific settings
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties(search PROPERTIES
        OSX_ARCHITECTURES "x86_64;arm64"
    )
endif()

# Add the tests
enable_testing()
add_executable(search_test search_test.cc)

foreach(test_name
        small_file
        empty_file
        final_partial_chunk
        across_chunks
        end_of_file
        no_double_count
        keyword_longer_than_chunk
        empty_keyword
        missing_file
        directory
        filename_argument
        too_many_arguments
        help_option
        unknown_option
        exact_whole_lines
        exact_no_trailing_newline
        exact_crlf
        exact_empty_file
        exact_chunk_boundaries
        exact_line_longer_than_chunk
        exact_keyword_longer_than_chunk)
    add_test(NAME ${test_name} COMMAND search_test $<TARGET_FILE:search> ${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 30)
endforeach()
