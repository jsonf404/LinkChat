cmake_minimum_required(VERSION 3.20)

project(LinkChat VERSION 1.0.0 LANGUAGES CXX)

option(LINKCHAT_BUILD_CLIENT "Build the Qt desktop client" ON)
option(LINKCHAT_BUILD_SERVERS "Build the C++ server applications" ON)
option(LINKCHAT_REGENERATE_PROTO "Regenerate protobuf and gRPC sources from message.proto" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(LINKCHAT_BUILD_CLIENT)
    add_subdirectory(client/linkchat)
endif()

if(LINKCHAT_BUILD_SERVERS)
    add_subdirectory(server)
endif()
