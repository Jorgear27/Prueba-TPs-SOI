#include "server.hpp"

Server::Server(Logger& logger, RequestRouter& router) : logger(logger), router(router)
{
}

Server::~Server()
{
}

bool Server::initialize()
{
    // Log the server initialization
    logger.log("Server", "[INFO] Initializing server...");
    logger.log("Server", "[INFO] Server initialized on port " + std::to_string(PORT));
    std::cout << "[INFO] Initializing server...\n";
    std::cout << "[INFO] Server initialized on port " << PORT << "\n";
    return true;
}

void Server::signalHandler(int signal)
{
    Logger::getInstance().log("System", "[INFO] Server shutting down due to signal: " + std::string(strsignal(signal)));
    exit(signal);
}

void Server::run()
{
    std::signal(SIGINT, signalHandler);  // Handle Ctrl+C
    std::signal(SIGTERM, signalHandler); // Handle termination signals

    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_fd, MAX_CONNECTIONS);

    // Log the server start
    logger.log("Server", "[INFO] Server is running and listening for connections...");
    std::cout << "[INFO] Server is running and listening for connections...\n";

    // Start the periodic thread for processing approved orders
    std::thread(&OrderManager::processApprovedOrders, OrderManager()).detach();

    while (running)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms timeout

        int activity = select(server_fd + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0 && errno != EINTR)
        {
            perror("select error");
            break;
        }

        if (activity > 0 && FD_ISSET(server_fd, &readfds))
        {
            client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

            // Handle the client in a new thread
            std::thread(&Server::handleClient, this, client_socket).detach();
        }
    }

    close(server_fd);
}

void Server::handleClient(int clientSocket)
{
    char buffer[BUFFER_SIZE] = {0};

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));

        // Read data from the client
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead <= 0)
        {
            if (bytesRead == 0)
            {
                // Client disconnected
                logger.log("Server", "[INFO] Client disconnected: " + std::to_string(clientSocket));
                std::cout << "[INFO] Client disconnected.\n";
            }
            else
            {
                // Error occurred
                logger.log("Server", "[INFO] Connection with client closed: " + std::to_string(clientSocket));
            }
            break;
        }

        std::string response;
        try
        {
            response = router.routeRequest(buffer, clientSocket); // Route the JSON request

            // If the response contains "disconnect", break the loop
            if (response.find("disconnect") != std::string::npos)
            {
                send(clientSocket, response.c_str(), response.size(), 0);
                logger.log("Server", "[INFO] Disconnecting client: " + std::to_string(clientSocket));
                std::cout << "[INFO] Disconnecting client: " << clientSocket << "\n";
                break;
            }
        }
        catch (const std::exception& e)
        {
            response = "{\"status\":\"error\",\"message\":\"Invalid request\"}";
        }

        // Send the response back to the client
        send(clientSocket, response.c_str(), response.size(), 0);

// Break the loop after processing one request during testing
#ifdef TESTING
        break;
#endif
    }

    close(clientSocket); // Close the client socket
}

void Server::stop()
{
    running = false;
    logger.log("Server", "[INFO] Stopping server...");
    std::cout << "[INFO] Stopping server...\n";
}

bool Server::isRunning()
{
    return running;
}
