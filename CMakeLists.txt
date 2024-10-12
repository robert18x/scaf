cmake_minimum_required(VERSION 3.26)

project(scaf LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No setting build type was specified. Setting Release build")
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json to make it easier to work with clang based tools

option(BUILD_TESTING "Enable tests" ON)

add_subdirectory(scaf)

if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(tests)
endif()

