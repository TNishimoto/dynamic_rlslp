cmake_minimum_required(VERSION 3.21)
project(dynamic_rlslp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CPP_STATIC)
set(CMAKE_CXX_FLAGS "-m64 -Wall -Wextra -static")
else(CPP_STATIC)
set(CMAKE_CXX_FLAGS "-m64 -Wall -Wextra")
endif(CPP_STATIC)



if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_definitions(RELEASE_BUILD)
else()
  add_compile_definitions(DEBUG_BUILD)
endif()


if(CPP_STATIC)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -static")
else(CPP_STATIC)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
endif(CPP_STATIC)

#set(CMAKE_CXX_FLAGS_DEBUG  "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-g -O0 -DDEBUG -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O0 -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE  "-O3 -DNDEBUG -DTIME_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3 -DNDEBUG -DNTIME_DEBUG")

INCLUDE_DIRECTORIES(./modules)

add_executable(build_dynamic_string main/build_dynamic_string_main.cpp)
add_executable(print_dynamic_string main/print_dynamic_string_main.cpp)


