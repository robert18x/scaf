cmake_minimum_required(VERSION 3.26)

project(scaf LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No setting build type was specified. Setting Release build")
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json to make it easier to work with clang based tools

option(TESTS_ENABLED "Enable tests" OFF)

add_library(${PROJECT_NAME} INTERFACE
  "include/AclMessage.h"
  "include/Agent.h"
  "include/Behaviour.h"
  "include/CommunicationHandler.h"
  "include/ConversationHandler.h"
  "include/Exceptions.h"
  "include/JsonSerializer.h"
  "include/Performative.h"
  "include/utils.h"
)

if(TESTS_ENABLED)


  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
      "${CMAKE_BINARY_DIR}/conan.cmake"
      EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
      TLS_VERIFY ON)
  endif()

  include(${CMAKE_BINARY_DIR}/conan.cmake)

  conan_cmake_autodetect(settings)


  conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}/conanfile.py
    BUILD missing
    SETTINGS ${settings})

  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

  add_executable("${PROJECT_NAME}_test"
    main.cpp
  )

  target_link_libraries("${PROJECT_NAME}_test" PUBLIC ${PROJECT_NAME})


endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
endif()

find_package(Threads)
find_package(nlohmann_json)
find_package(fmt)

target_link_libraries(${PROJECT_NAME} INTERFACE
  fmt::fmt
  nlohmann_json::nlohmann_json
  Threads::Threads
)

