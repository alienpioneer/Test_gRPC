#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "test.grpc.pb.h"

std::string server_addr = "127.0.0.1:50051";

class TestClient {
public:
    TestClient(std::shared_ptr<grpc::Channel> channel)
        : m_grpc_stub(test::TestService::NewStub(channel)) {}

    std::string rpcTest(const std::string user)
    {
        // Data we are sending to the server.
        test::TestRequest request;
        request.set_name(user);

        // Container for the data we expect from the server.
        test::TestReply reply;

        // Context for the client. It could be used to convey extra information to0
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        grpc::CompletionQueue queue;
        // The actual async RPC
        std::unique_ptr<grpc::ClientAsyncResponseReader<test::TestReply> > rpc(m_grpc_stub->AsyncTestMessage(&context, request, &queue));

        grpc::Status status;
        std::cout << "Sending request: "  << request.name() << "\n";
        rpc->Finish(&reply, &status, (void*)1);

        void* got_tag;
        bool ok = false;
        queue.Next(&got_tag, &ok);

        if (ok && got_tag == (void*)1)
        {
            // Act upon its status.
            if (status.ok())
            {
                return reply.message();
            }
            else
            {
                std::cout << status.error_code() << ": " << status.error_message() << "\n";
                return "RPC failed";
            }
        }
    }

private:
    std::unique_ptr<test::TestService::Stub> m_grpc_stub;
};

int main(int argc, char** argv)
{
    std::cout << "Connecting to : " << server_addr << std::endl;

    TestClient client(grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials()));

    std::string reply = client.rpcTest("test_async_client");
    std::cout << "Received reply: " << reply << std::endl;

    std::getchar();

    return 0;
}