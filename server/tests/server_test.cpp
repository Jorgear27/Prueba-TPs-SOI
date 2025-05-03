#include "server.hpp"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>

// Test for server initialization
TEST(ServerTest, Initialize)
{
    Server server;
    EXPECT_TRUE(server.initialize());
}

// Test for handling a client (mocked)
TEST(ServerTest, HandleClient)
{
    Server server;

    // Mock a client socket (use a pipe to simulate communication)
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    std::string request = R"({
        "type": "client_info",
        "timestamp": "2025-04-01T12:10:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";

    write(pipefd[1], request.c_str(), request.size());
    close(pipefd[1]);

    EXPECT_NO_THROW(server.handleClient(pipefd[0]));
    close(pipefd[0]);
}

// Test for handling client disconnection
TEST(ServerTest, HandleClientDisconnection)
{
    Server server;

    // Mock a client socket (use a pipe to simulate communication)
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    close(pipefd[1]); // Simulate client disconnection

    server.handleClient(pipefd[0]);
    close(pipefd[0]);

    // Check if the server handles disconnection gracefully
    // (No exceptions should be thrown)
    EXPECT_NO_THROW(server.handleClient(pipefd[0]));
}
