cmake_minimum_required(VERSION 3.26)

project(scaf_tests)


add_executable(${PROJECT_NAME}
  main.cpp
)

find_package(Threads)
find_package(nlohmann_json)
find_package(fmt)

target_include_directories(${PROJECT_NAME} PUBLIC . ../scaf)
target_link_libraries(${PROJECT_NAME} PUBLIC
  scaf
  fmt::fmt
  nlohmann_json::nlohmann_json
  Threads::Threads
)
