project(scaf)

add_library(${PROJECT_NAME} INTERFACE
  AclMessage.h
  Agent.h
  Behaviour.h
  CommunicationHandler.h
  ConversationHandler.h
  Exceptions.h
  JsonSerializer.h
  Performative.h
  utils.h
)

find_package(Threads)
find_package(nlohmann_json)
find_package(fmt)

target_link_libraries(${PROJECT_NAME} INTERFACE
  fmt::fmt
  nlohmann_json::nlohmann_json
  Threads::Threads
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
endif()