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
	AsyncRpcRequest(test::TestService::AsyncService* service, grpc::ServerCompletionQueue* serverQueue)
		: m_responder(&m_context), m_status (Status::CREATE)
	{
		m_tag = std::chrono::steady_clock::now().time_since_epoch().count();

		service->RequestTestMessage(&m_context, &m_request, &m_responder, serverQueue, serverQueue, (void*)m_tag);

		m_status = Status::PROCESS;
	}

	bool processRpcRequest()
	{
		if (m_status == Status::PROCESS)
		{
			test::TestReply response;
			std::cout << "Received request from " << m_request.name() << "\n";
			std::string prefix("Hello ");
			response.set_message(prefix + m_request.name());

			m_responder.Finish(response, grpc::Status::OK, (void*)m_tag);
			m_status = Status::FINISH;
			return false;
		}
		else if (m_status == Status::FINISH)
		{
			std::cout << "Finnished request from " << m_request.name() << "\n";
			return true;
		}
	}

	const long long getTag() const
	{
		return m_tag;
	}

private:
	grpc::ServerContext									m_context;
	test::TestRequest									m_request;
	grpc::ServerAsyncResponseWriter<test::TestReply>	m_responder;
	long long											m_tag;
	enum class Status { CREATE, PROCESS, FINISH }		m_status;
};

class AsyncServer final 
{	
public:
	AsyncServer(){}
	~AsyncServer()
	{
		m_server->Shutdown();
		m_serverQueue->Shutdown();
	}

	void run() 
	{
		grpc::ServerBuilder builder;

		grpc::EnableDefaultHealthCheckService(true);
		grpc::reflection::InitProtoReflectionServerBuilderPlugin();

		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

		builder.RegisterService(&m_asyncService);

		m_serverQueue = builder.AddCompletionQueue();

		// Finally assemble the server.
		m_server = builder.BuildAndStart();
		std::cout << "Server listening on " << server_address << std::endl;

		// Proceed to the server's main loop.
		HandleRpcRequests();
	}

	// This can be run in multiple threads if needed.
	void HandleRpcRequests()
	{
		void* tag;  // uniquely identifies a request.
		bool ok;
		createNewRequest();

		while (true) {
			// Block waiting to read the next event from the completion queue.
			// The return value of Next should always be checked. This return value
			// tells us whether there is any kind of event or m_serverQueue is shutting down.
			if (m_serverQueue->Next(&tag, &ok) && ok)
			{
				if ( m_requestList.find((long long)tag) != m_requestList.end())
				{
					// If true, the AsyncRpcRequest is finnished and ok to delete
					if (m_requestList.at((long long)tag)->processRpcRequest())
					{
						delete m_requestList.at((long long)tag);
						m_requestList.erase((long long)tag);
					}
				}
				else
				{
					createNewRequest();
				}
			}
		}
	}

	void createNewRequest()
	{
		AsyncRpcRequest* request = new AsyncRpcRequest( &m_asyncService, m_serverQueue.get() );
		m_requestList[request->getTag()] = request;
	}

private:
	std::unique_ptr<grpc::ServerCompletionQueue>	m_serverQueue;
	test::TestService::AsyncService					m_asyncService;
	std::unique_ptr<grpc::Server>					m_server;
	std::unordered_map<long long, AsyncRpcRequest*>	m_requestList;
};


int main(int argc, char** argv)
{
	AsyncServer server;
	server.run();
	return 0;
}