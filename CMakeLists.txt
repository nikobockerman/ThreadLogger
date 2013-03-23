cmake_minimum_required(VERSION 2.8.8)
project(ThreadLogger)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif (NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_FLAGS "-std=c++0x -pedantic -Wall -Wextra -Werror -pthread" CACHE STRING
    "g++ Compiler Flags for All Builds" FORCE)
set (CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING
    "g++ Compiler Flags for Debug Builds" FORCE)

set (CMAKE_VERBOSE_MAKEFILE on)

find_package(Qt5Core REQUIRED)

SET(LIBRARY_SOURCES
    threadlogger.h threadlogger.cpp
    compiler_check.h
)

add_library (threadlogger ${LIBRARY_SOURCES})
qt5_use_modules(threadlogger Core)
