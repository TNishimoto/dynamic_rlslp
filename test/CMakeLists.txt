cmake_minimum_required(VERSION 3.21)
project(test_dynamic_string_constructor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_definitions(RELEASE_BUILD)
else()
  add_compile_definitions(DEBUG_BUILD)
endif()

#set(CMAKE_CXX_FLAGS_DEBUG  "-g -O0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-g -O0 -DDEBUG -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3 -DNDEBUG")

# インクルードディレクトリを追加
INCLUDE_DIRECTORIES(../modules)
INCLUDE_DIRECTORIES(../include)

add_executable(test_dynamic_string_constructor test_dynamic_string_constructor.cpp)
add_executable(test_dynamic_string_access test_dynamic_string_access.cpp)
add_executable(test_dynamic_string_lce test_dynamic_string_lce.cpp)
add_executable(test_dynamic_string_reverse_lce test_dynamic_string_reverse_lce.cpp)
add_executable(test_dynamic_string_get_all_occurrences test_dynamic_string_get_all_occurrences.cpp)
add_executable(test_dynamic_string_lcp test_dynamic_string_lcp.cpp)
add_executable(test_dynamic_string_lcs test_dynamic_string_lcs.cpp)
add_executable(test_dynamic_string_insert test_dynamic_string_insert.cpp)
add_executable(test_dynamic_string_delete test_dynamic_string_delete.cpp)
add_executable(test_dynamic_string_file_io test_dynamic_string_file_io.cpp)
add_executable(test_dynamic_string_random_ops test_dynamic_string_random_ops.cpp)

