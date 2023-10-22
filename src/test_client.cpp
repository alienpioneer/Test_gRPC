#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <grpcpp/grpcpp.h>
#include "test.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

ABSL_FLAG(std::string, server, "127.0.0.1:50051", "Server address");

class TestClient {
public:
    TestClient(std::shared_ptr<grpc::Channel> channel)
        : m_grpc_stub(test::TestService::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string rpcTest(const std::string& user) 
    {
        // Data we are sending to the server.
        test::TestRequest request;
        request.set_name(user);

        // Container for the data we expect from the server.
        test::TestReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        grpc::Status status = m_grpc_stub->TestMessage(&context, request, &reply);

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

private:
    std::unique_ptr<test::TestService::Stub> m_grpc_stub;
};

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint specified by
    // the argument "--target=" which is the only expected argument.
    std::string target_str = absl::GetFlag(FLAGS_server);
    std::cout << "Target: " << target_str << std::endl;
    // We indicate that the channel isn't authenticated (use of
    // InsecureChannelCredentials()).
    TestClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::string user("test_client");
    std::string reply = client.rpcTest(user);
    std::cout << "Received reply: " << reply << std::endl;

    std::getchar();

    return 0;
}