cmake_minimum_required(VERSION 3.15)
project(ChatApp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME nlohmann_json
  GITHUB_REPOSITORY nlohmann/json
  GIT_TAG        v3.12.0
)

add_subdirectory(third_party)

add_library(server_lib
  Server/src/Server.cpp
  Server/src/CommandRouter.cpp
  Server/src/TopicManager.cpp
  Server/src/UserManager.cpp
)
target_include_directories(server_lib PUBLIC Server/include)
target_link_libraries(server_lib
  PRIVATE
    uwebsockets
    nlohmann_json::nlohmann_json
)

add_executable(chat_server Server/src/main.cpp)
target_link_libraries(chat_server PRIVATE server_lib)