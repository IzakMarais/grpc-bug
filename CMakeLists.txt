cmake_minimum_required(VERSION 3.12)
project(cnc)

set(VCPKG_ROOT $ENV{VCPKG_ROOT})
message("VCPKG ROOT: " ${VCPKG_ROOT})

# release or debug
if(CMAKE_BUILD_TYPE STREQUAL "Release")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(".")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT DEFINED ENV{Flatbuffers_ROOT})
        message(FATAL_ERROR "Flatbuffers_ROOT environment variable not defined")
    endif()
    set(Flatbuffers_ROOT $ENV{Flatbuffers_ROOT})
    message("Flatbuffers ROOT: " ${Flatbuffers_ROOT})
endif()

# compile .proto files to access etcd via V3 API
include(FindProtobuf)
find_package(Protobuf CONFIG REQUIRED)

message(STATUS "Using protobuf compiler: " ${Protobuf_PROTOC_EXECUTABLE})
message(STATUS "Protobuf_INCLUDE_DIR: " ${Protobuf_INCLUDE_DIR})
message(STATUS "Protobuf_LIBRARIES: " ${Protobuf_LIBRARIES})

include_directories(${Protobuf_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR})

set(proto_files route_guide.proto)


set(GOOGLE_ROOT ${VCPKG_ROOT}/installed/x64-linux/include)

# each file
FOREACH(proto_file ${proto_files})
    execute_process(COMMAND ${Protobuf_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_BINARY_DIR} -I${GOOGLE_ROOT} -I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${proto_file} ERROR_VARIABLE err)
message(STATUS "protoc err = "  ${err})
ENDFOREACH(proto_file)

#gRPC
find_package(gRPC CONFIG REQUIRED)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(GRPC_CPP_EXEC ${VCPKG_ROOT}/installed/x86-windows/tools/grpc/grpc_cpp_plugin.exe)
else()
    set(GRPC_CPP_EXEC ${VCPKG_ROOT}/installed/x64-linux/bin/grpc_cpp_plugin)
endif()

execute_process(COMMAND ${Protobuf_PROTOC_EXECUTABLE} --grpc_out=${CMAKE_BINARY_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_EXEC} -I${CMAKE_CURRENT_SOURCE_DIR} -I${GOOGLE_ROOT} ${CMAKE_CURRENT_SOURCE_DIR}/route_guide.proto ERROR_VARIABLE err)

message(STATUS ${err})

set(PROTO_HEADER ${CMAKE_BINARY_DIR}/route_guide.pb.h
				 ${CMAKE_BINARY_DIR}/route_guide.grpc.pb.h)

set(PROTO_SRC    ${CMAKE_BINARY_DIR}/route_guide.pb.cc
				 ${CMAKE_BINARY_DIR}/route_guide.grpc.pb.cc)

add_library(proto STATIC ${PROTO_HEADER} ${PROTO_SRC})


include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1 -DUSE_AVX -DWITH_AVX2 -DMBCS)

set(CLIENT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/route_guide_client.cc)

add_executable(route_guide_client ${CLIENT_SOURCES})
target_link_libraries(route_guide_client PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++)
target_link_libraries(route_guide_client PRIVATE
                      proto
                      ${Protobuf_LIBRARIES})

set(SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/route_guide_server.cc)

add_executable(route_guide_server ${SERVER_SOURCES})
target_link_libraries(route_guide_server PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc_cronet)
target_link_libraries(route_guide_server PRIVATE
                      proto
                      ${Protobuf_LIBRARIES})

