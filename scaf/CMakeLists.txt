project(scaf)

add_library(${PROJECT_NAME}
  AclMessage.h
  Agent.h
  Behaviour.h
  CommunicationHandler.h
  ConversationHandler.h
  Error.h
  ErrorHandler.h
  JsonSerializer.h
  Performative.h
  SynchronizedMap.h
  ThreadManager.h
  Uid.h
  utils.h
  empty.cpp
)

find_package(nlohmann_json)
find_package(fmt)
find_package(bshoshany-thread-pool)

target_link_libraries(${PROJECT_NAME} PUBLIC
  fmt::fmt
  nlohmann_json::nlohmann_json
  bshoshany-thread-pool::bshoshany-thread-pool
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
