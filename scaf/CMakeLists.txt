project(scaf)

add_library(${PROJECT_NAME} INTERFACE
  AclMessage.h
  Agent.h
  Behaviour.h
  CommunicationHandler.h
  ConversationHandler.h
  Error.h
  ErrorHandler.h
  JsonSerializer.h
  Performative.h
  Uid.h
  utils.h
)

find_package(nlohmann_json)
find_package(fmt)
find_package(magic_enum)

target_link_libraries(${PROJECT_NAME} INTERFACE
  fmt::fmt
  nlohmann_json::nlohmann_json
  magic_enum::magic_enum
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
endif()
