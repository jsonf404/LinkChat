find_package(Boost 1.70 REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(Threads REQUIRED)

set(LINKCHAT_MYSQL_ROOT "" CACHE PATH "MySQL Connector/C++ installation prefix")
set(LINKCHAT_HIREDIS_ROOT "" CACHE PATH "hiredis installation prefix")

find_path(LINKCHAT_MYSQL_INCLUDE_DIR jdbc/mysql_driver.h
    HINTS "${LINKCHAT_MYSQL_ROOT}" PATH_SUFFIXES include)
find_library(LINKCHAT_MYSQL_LIBRARY NAMES mysqlcppconn mysqlcppconn8
    HINTS "${LINKCHAT_MYSQL_ROOT}" PATH_SUFFIXES lib lib64)
find_path(LINKCHAT_HIREDIS_INCLUDE_DIR hiredis.h
    HINTS "${LINKCHAT_HIREDIS_ROOT}" PATH_SUFFIXES include include/hiredis)
find_library(LINKCHAT_HIREDIS_LIBRARY NAMES hiredis
    HINTS "${LINKCHAT_HIREDIS_ROOT}" PATH_SUFFIXES lib lib64)

if(NOT LINKCHAT_MYSQL_INCLUDE_DIR OR NOT LINKCHAT_MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL Connector/C++ not found. Set LINKCHAT_MYSQL_ROOT.")
endif()
if(NOT LINKCHAT_HIREDIS_INCLUDE_DIR OR NOT LINKCHAT_HIREDIS_LIBRARY)
    message(FATAL_ERROR "hiredis not found. Set LINKCHAT_HIREDIS_ROOT.")
endif()

if(TARGET JsonCpp::JsonCpp)
    set(LINKCHAT_JSONCPP_TARGET JsonCpp::JsonCpp)
elseif(TARGET jsoncpp_lib)
    set(LINKCHAT_JSONCPP_TARGET jsoncpp_lib)
else()
    message(FATAL_ERROR "JsonCpp package did not provide a supported CMake target.")
endif()

add_library(linkchat_server_common INTERFACE)
if(TARGET Boost::headers)
    set(LINKCHAT_BOOST_TARGET Boost::headers)
else()
    set(LINKCHAT_BOOST_TARGET Boost::boost)
endif()
target_include_directories(linkchat_server_common INTERFACE
    "${LINKCHAT_MYSQL_INCLUDE_DIR}"
    "${LINKCHAT_HIREDIS_INCLUDE_DIR}"
)
target_link_libraries(linkchat_server_common INTERFACE
    ${LINKCHAT_BOOST_TARGET}
    gRPC::grpc++
    protobuf::libprotobuf
    ${LINKCHAT_JSONCPP_TARGET}
    "${LINKCHAT_MYSQL_LIBRARY}"
    "${LINKCHAT_HIREDIS_LIBRARY}"
    Threads::Threads
)
if(WIN32)
    target_link_libraries(linkchat_server_common INTERFACE ws2_32 mswsock)
endif()

function(linkchat_add_server target source_dir)
    file(GLOB _sources CONFIGURE_DEPENDS "${source_dir}/*.cpp" "${source_dir}/*.cc")
    add_executable(${target} ${_sources})
    target_include_directories(${target} PRIVATE "${source_dir}")
    target_link_libraries(${target} PRIVATE linkchat_server_common)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /wd4819)
    endif()
endfunction()

if(LINKCHAT_REGENERATE_PROTO)
    get_target_property(_protoc_location protobuf::protoc IMPORTED_LOCATION)
    get_target_property(_grpc_plugin_location gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
    if(NOT _protoc_location OR NOT _grpc_plugin_location)
        message(FATAL_ERROR "Could not resolve protoc and grpc_cpp_plugin executable locations.")
    endif()
    foreach(_dir GateServer StatusServer ChatServer ChatServer2)
        set(_proto_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}")
        add_custom_command(
            OUTPUT "${_proto_dir}/message.pb.cc" "${_proto_dir}/message.pb.h"
                   "${_proto_dir}/message.grpc.pb.cc" "${_proto_dir}/message.grpc.pb.h"
            COMMAND "${_protoc_location}"
                --proto_path="${_proto_dir}"
                --cpp_out="${_proto_dir}"
                --grpc_out="${_proto_dir}"
                --plugin=protoc-gen-grpc="${_grpc_plugin_location}"
                "${_proto_dir}/message.proto"
            DEPENDS "${_proto_dir}/message.proto"
            VERBATIM
        )
    endforeach()
endif()

linkchat_add_server(GateServer "${CMAKE_CURRENT_SOURCE_DIR}/GateServer")
linkchat_add_server(StatusServer "${CMAKE_CURRENT_SOURCE_DIR}/StatusServer")
linkchat_add_server(ChatServer "${CMAKE_CURRENT_SOURCE_DIR}/ChatServer")
linkchat_add_server(ChatServer2 "${CMAKE_CURRENT_SOURCE_DIR}/ChatServer2")
