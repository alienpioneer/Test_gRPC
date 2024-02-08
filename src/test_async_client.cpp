#include <iostream>
#include <memory>
#include <string>
#include <array>

#include <grpcpp/grpcpp.h>
#include "test.grpc.pb.h"

std::string server_addr = "127.0.0.1:50051";

class TestClient 
{
public:
    TestClient(std::shared_ptr<grpc::Channel> channel)
        : m_grpc_stub(test::TestService::NewStub(channel)) {}

    void rpcTest(const std::string user)
    {
        // Data we are sending to the server.
        test::TestRequest request;
        request.set_name(user);

        // Container for the data we expect from the server.
        AsyncReply* asyncReply = new AsyncReply();

        // The actual async RPC
        std::unique_ptr<grpc::ClientAsyncResponseReader<test::TestReply> > responseReader =
            m_grpc_stub->PrepareAsyncTestMessage(&asyncReply->context, request, &m_queue);

        std::cout << "Sending request: "  << request.name() << "\n";
        responseReader->StartCall();
        responseReader->Finish(&asyncReply->reply, &asyncReply->status, (void*)asyncReply);
    }

    void processQueue()
    {
        void* received_tag;
        bool received_ok = false;

        while (m_queue.Next(&received_tag, &received_ok))
        {
            if (received_ok)
            {
                AsyncReply* reply = static_cast<AsyncReply*>(received_tag);
                // Act upon its status.
                if (reply->status.ok())
                {
                    std::cout << "Received reply: " << reply->reply.message() << std::endl;
                }
                else
                {
                    std::cout << "RPC failed  " << reply->status.error_code() << ": " << reply->status.error_message() << "\n";
                }
            }
        }
    }

    static void checkConnection(std::shared_ptr<grpc::Channel> connection)
    {
        grpc_connectivity_state connectionState = GRPC_CHANNEL_IDLE;
        grpc_connectivity_state connectionPrevState = GRPC_CHANNEL_TRANSIENT_FAILURE;

        std::array<std::string, 5> connectionToString = { "GRPC_CHANNEL_IDLE",
                                                        "GRPC_CHANNEL_CONNECTING",
                                                        "GRPC_CHANNEL_READY",
                                                        "GRPC_CHANNEL_TRANSIENT_FAILURE",
                                                        "GRPC_CHANNEL_SHUTDOWN" };

        while (connectionState != GRPC_CHANNEL_SHUTDOWN)
        {
            connectionState = connection->GetState(false);

            if (connectionPrevState != connectionState)
            {
                std::cout << "Connection state : " << connectionToString[connectionState] << "\n";
                connectionPrevState = connectionState;
            }
        }
    }

private:
    struct AsyncReply 
    {
        grpc::Status        status;
        test::TestReply     reply;
        grpc::ClientContext context;
    };

private:
    std::unique_ptr<test::TestService::Stub>    m_grpc_stub;
    grpc::CompletionQueue                       m_queue;
};

int main(int argc, char** argv)
{
    std::shared_ptr<grpc::Channel> connection = grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials());

    std::cout << "Connecting to : " << server_addr << "\n";

    TestClient client(connection);

    std::thread mThread = std::thread(&TestClient::processQueue, &client);
    std::thread cThread = std::thread(&TestClient::checkConnection, connection);

    for (auto i = 1; i < 5; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        client.rpcTest("test_async_client"+std::to_string(i));
    }
    
    mThread.join();
    cThread.join();

    return 0;
}