cmake_minimum_required(VERSION 2.8.8)
project(ThreadLogger)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Whether to support Qt5 or not
option(USE_QT5 "Use Qt5" ON)

# Compile target. Default to Debug.
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# Compile options.
set (CMAKE_CXX_FLAGS "-std=c++0x -pedantic -Wall -Wextra -Werror -pthread" CACHE STRING
    "g++ Compiler Flags for All Builds" FORCE)
set (CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING
    "g++ Compiler Flags for Debug Builds" FORCE)

# Force cmake to create verbose makefile
set (CMAKE_VERBOSE_MAKEFILE on)

SET(LIBRARY_SOURCES
    threadlogger.h threadlogger.cpp
    loggable.h
    compiler_check.h
)

add_library (threadlogger ${LIBRARY_SOURCES})

# Qt5 library
if (USE_QT5)
    find_package(Qt5Core REQUIRED)
    qt5_use_modules(threadlogger Core)
    message(STATUS "Qt5 dependency is used.")
else (USE_QT5)
    message(FATAL_ERROR "Qt5 is currently not an optional dependency.")
endif (USE_QT5)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)
