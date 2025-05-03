#include "warehouse.h"

int connect_wh_to_server()
{
    int sock;
    struct sockaddr_in server_addr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(sock);
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection to the server failed");
        close(sock);
        return -1;
    }
    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1)
    {
        perror("[ERROR] Failed to get socket flags");
        return -1;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("[ERROR] Failed to set socket to non-blocking mode");
        return -1;
    }

    return sock;
}

void disconnect_wh_from_server(int sock, const char* warehouse_id)
{
    // Create a JSON object for disconnect request
    cJSON* json = cJSON_CreateObject();
    if (json == NULL)
    {
        perror("Failed to create JSON object");
        close(sock);
        return;
    }

    // Generate a timestamp
    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_wh(timestamp, sizeof(timestamp));

    // Add disconnect request type to JSON object
    cJSON_AddStringToObject(json, "type", "disconnect_request");
    cJSON_AddStringToObject(json, "user_id", warehouse_id);
    cJSON_AddStringToObject(json, "timestamp", timestamp);

    // Convert JSON object to string
    char* disconnect_message = cJSON_PrintUnformatted(json);
    if (disconnect_message == NULL)
    {
        perror("Failed to print JSON object");
        cJSON_Delete(json);
        close(sock);
        return;
    }

    // Send disconnect message to server
    if (send_message_from_wh(sock, disconnect_message) < 0)
    {
        perror("Failed to send disconnect message");
    }

    // Free the JSON string and object
    free(disconnect_message);
    cJSON_Delete(json);

    // Close the socket
    close(sock);
    printf("Disconnected from server\n");
    exit(0);
}

int send_message_from_wh(int sock, const char* message)
{
    return send(sock, message, strlen(message), 0);
}

int receive_message_wh(int sock, char* buffer, size_t buffer_size)
{
    return recv(sock, buffer, buffer_size - 1, 0);
}
