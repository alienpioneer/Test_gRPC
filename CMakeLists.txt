cmake_minimum_required(VERSION 3.8)

project( Test_grpc C CXX)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

# -------------------  Setup paths
include_directories("./proto")
set(PROTO_FILE_NAME "test")

set(CMAKE_MODULE_PATH "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/;C:/Program Files (x86)/grpc/")
set(CMAKE_PREFIX_PATH "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/;C:/Program Files (x86)/grpc/")
set(CMAKE_PROGRAM_PATH "D:/_DEV_PROJECTS/DEV_LIBRARIES/build_grpc/install/bin")

find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

find_package(Threads REQUIRED)

# Find Protobuf installation - Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# Find gRPC installation - Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)

# -------------------  Generate sources from proto file
get_filename_component(proto "proto/${PROTO_FILE_NAME}.proto" ABSOLUTE)
get_filename_component(proto_path "${proto}" PATH)

set(proto_srcs "../../../proto/${PROTO_FILE_NAME}.pb.cc")
set(proto_hdrs "../../../proto/${PROTO_FILE_NAME}.pb.h")
set(grpc_srcs "../../../proto/${PROTO_FILE_NAME}.grpc.pb.cc")
set(grpc_hdrs "../../../proto/${PROTO_FILE_NAME}.grpc.pb.h")

add_custom_command(
      OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "../../../proto"
           --cpp_out "../../../proto"
           -I "${proto_path}"
           --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
           "${proto}"
      DEPENDS "${proto}")
# -------------------  Generate sources from proto file END
# grpc_proto
add_library(grpc_proto ${grpc_srcs} ${grpc_hdrs} ${proto_srcs} ${proto_hdrs})
target_link_libraries(grpc_proto ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

# Build sources
add_executable( test_server "src/test_server.cpp")
target_link_libraries(test_server grpc_proto absl::flags absl::flags_parse ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

add_executable( test_client "src/test_client.cpp")
target_link_libraries(test_client grpc_proto absl::flags absl::flags_parse ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})