#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "test.grpc.pb.h"

class ServiceImpl final : public test::TestService:: Service
{
	virtual ::grpc::Status TestMessage(grpc::ServerContext* context, const test::TestRequest* request, test::TestReply* response) override
	{
		std::cout << "Received request from " << request->name();
		std::string prefix("Hello ");
		response->set_message(prefix + request->name());
		return grpc::Status::OK;
	}
};

void RunServer() {
	std::string server_address = "127.0.0.1:50051";
	ServiceImpl service;

	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();

	grpc::ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);

	// Finally assemble the server.
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}

int main(int argc, char** argv) {
	RunServer();
	return 0;
}