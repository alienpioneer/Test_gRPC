cmake_minimum_required(VERSION 3.8)

project( Test_grpc C CXX)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

# -------------------  Setup paths
include_directories("./proto")

set(CMAKE_MODULE_PATH "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/")
set(CMAKE_PREFIX_PATH "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/")
set(CMAKE_PROGRAM_PATH "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/bin")

find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

find_package(Threads REQUIRED)

# -------------------  Generate stubs from proto file
get_filename_component(hw_proto "proto/test.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

set(hw_proto_srcs "../../../proto/test.pb.cc")
set(hw_proto_hdrs "../../../proto/test.pb.h")
set(hw_grpc_srcs "../../../proto/test.grpc.pb.cc")
set(hw_grpc_hdrs "../../../proto/test.grpc.pb.h")

add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "../../../proto"
           --cpp_out "../../..//proto"
           -I "${hw_proto_path}"
           --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
           "${hw_proto}"
      DEPENDS "${hw_proto}")
# -------------------  Generate stubs from proto file END

# Find Protobuf installation - Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)


#include_directories("./proto" "C:/Program Files (x86)/grpc/include" "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/include")

# hw_grpc_proto
add_library(hw_grpc_proto ${hw_grpc_srcs} ${hw_grpc_hdrs} ${hw_proto_srcs} ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

add_executable( test_server "src/test_server.cpp")
target_link_libraries(test_server hw_grpc_proto absl::flags absl::flags_parse ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})