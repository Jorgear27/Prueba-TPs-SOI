#include "authentication.hpp"
#include "database.hpp"
#include "inventory.hpp"
#include "log.hpp"
#include "orders.hpp"
#include "request_router.hpp"
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

    // Create dependencies for the Server
    Logger& logger = Logger::getInstance();
    Sender& sender = Sender::getInstance();
    Authentication auth(db, sender);
    InventoryManager inventoryManager;
    OrderManager orderManager;
    RequestRouter router(auth, inventoryManager, orderManager);

    // Create the Server instance with the required dependencies
    Server server(logger, router);

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
