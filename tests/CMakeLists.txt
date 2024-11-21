cmake_minimum_required(VERSION 3.26)

project(scaf_tests)


add_executable(${PROJECT_NAME}
  main.cpp
)

find_package(Threads)
find_package(nlohmann_json)
find_package(fmt)
find_package(magic_enum)

target_include_directories(${PROJECT_NAME} PUBLIC . ../scaf)
target_link_libraries(${PROJECT_NAME} PUBLIC
  scaf
  fmt::fmt
  nlohmann_json::nlohmann_json
  magic_enum::magic_enum
  Threads::Threads
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
