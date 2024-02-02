#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "test.grpc.pb.h"

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
                std::cout << status.error_code() << ": " << status.error_message() << std::endl;
                return "RPC failed";
            }
        }
    }

private:
    std::unique_ptr<test::TestService::Stub> m_grpc_stub;
};

int main(int argc, char** argv)
{
    std::string target_str = "127.0.0.1:50051";
    std::cout << "Target: " << target_str << std::endl;

    TestClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    std::string reply = client.rpcTest("test_async_client");
    std::cout << "Received reply: " << reply << std::endl;

    std::getchar();

    return 0;
}