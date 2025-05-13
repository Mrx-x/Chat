cmake_minimum_required(VERSION 3.15)
project(uwebsockets_header_only LANGUAGES CXX)

add_library(uwebsockets INTERFACE)

target_include_directories(uwebsockets INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/uWebSockets/src
    ${CMAKE_CURRENT_LIST_DIR}/uWebSockets/include
)