#include "database.hpp"
#include "log.hpp"
#include "server.hpp"
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

int main()
{
    // Connect to the DB
    Database& db = Database::getInstance();

    if (!db.is_connected())
    {
        return EXIT_FAILURE;
    }

    Server server;

    // Initialize the server
    if (!server.initialize())
    {
        Logger::getInstance().log("Main", "[ERROR] Failed to initialize the server.");
        std::cerr << "[ERROR] Failed to initialize the server.\n";
        return EXIT_FAILURE;
    }

    server.run();

    Logger::getInstance().log("Main", "[INFO] Server started successfully.");
    std::cout << "[INFO] Server started successfully.\n";
    // The server will run indefinitely, handling incoming connections
    // and processing requests in the run() method.

    return EXIT_SUCCESS;
}
