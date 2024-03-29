#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <unordered_map>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "test.grpc.pb.h"

std::string server_address = "127.0.0.1:50051";

class AsyncRpcRequest
{
public:
	AsyncRpcRequest(test::TestService::AsyncService* service, grpc::ServerCompletionQueue& serverQueue)
		: m_responder(&m_context), m_status (Status::CREATE)
	{
		service->RequestTestMessage(&m_context, &m_request, &m_responder, &serverQueue, &serverQueue, (void*)this);

		m_status = Status::PROCESS;
	}

	bool processRpcRequest()
	{
		if (m_status == Status::PROCESS)
		{
			test::TestReply response;
			std::cout << "Received request \"" << m_request.name() << "\"\n";
			std::string prefix("Hello ");
			response.set_message(prefix + m_request.name());

			// Delay the response
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));

			m_responder.Finish(response, grpc::Status::OK, (void*)this);
			m_status = Status::FINISH;
			return false;
		}
		else if (m_status == Status::FINISH)
		{
			std::cout << "Finnished the request \"" << m_request.name() << "\"\n";
			return true;
		}
	}

private:
	grpc::ServerContext									m_context;
	test::TestRequest									m_request;
	grpc::ServerAsyncResponseWriter<test::TestReply>	m_responder;
	enum class Status { CREATE, PROCESS, FINISH }		m_status;
};

class AsyncServer final 
{	
public:
	AsyncServer()
	{
		grpc::EnableDefaultHealthCheckService(true);
		grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	}

	~AsyncServer()
	{
		m_server->Shutdown();
		m_serverQueue->Shutdown();
	}

	void run() 
	{
		grpc::ServerBuilder builder;

		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

		builder.RegisterService(&m_asyncService);

		m_serverQueue = builder.AddCompletionQueue();

		// Finally assemble the server.
		m_server = builder.BuildAndStart();
		std::cout << "Server listening on " << server_address << std::endl;

		// Create a default request reservation
		new AsyncRpcRequest(&m_asyncService, *m_serverQueue);
		handleRpcRequests();
	}

	// This can be run in multiple threads if needed.
	void handleRpcRequests()
	{
		void* tag;  // uniquely identifies a request.
		bool ok;

		// Block waiting to read the next event from the completion queue.
		// The return value of Next should always be checked. This return value
		// tells us whether there is any kind of event or m_serverQueue is shutting down.
		while (m_serverQueue->Next(&tag, &ok))
		{
			AsyncRpcRequest* request = static_cast<AsyncRpcRequest*>(tag);

				// If true, the AsyncRpcRequest is finnished and ok to delete
			if (request->processRpcRequest())
			{
				delete request;
			}
			else
			{
				new AsyncRpcRequest(&m_asyncService, *m_serverQueue);
			}
		}
	}

private:

	void createNewServiceRequest()
	{
		
	}

private:
	std::unique_ptr<grpc::ServerCompletionQueue>	m_serverQueue;
	test::TestService::AsyncService					m_asyncService;
	std::unique_ptr<grpc::Server>					m_server;
};


int main(int argc, char** argv)
{
	AsyncServer server;
	server.run();
	return 0;
}