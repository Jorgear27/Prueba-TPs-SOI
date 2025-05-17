#include "hub.h"

int connect_hub_to_server()
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
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    int flags = fcntl(sock, F_GETFL, 0);

    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    return sock;
}

void disconnect_hub_from_server(int sock, const char* hub_id)
{
    // Create a JSON object for disconnect request
    cJSON* json = cJSON_CreateObject();

    // Generate a timestamp
    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_hub(timestamp, sizeof(timestamp));

    // Add disconnect request type to JSON object
    cJSON_AddStringToObject(json, "type", "disconnect_request");
    cJSON_AddStringToObject(json, "user_id", hub_id);
    cJSON_AddStringToObject(json, "timestamp", timestamp);

    // Convert JSON object to string
    char* disconnect_message = cJSON_PrintUnformatted(json);

    // Send disconnect message to server
    send_message_from_hub(sock, disconnect_message);

    // Free the JSON string and object
    free(disconnect_message);
    cJSON_Delete(json);

    // Close the socket
    close(sock);
    printf("Disconnected from server\n");
}

int send_message_from_hub(int sock, const char* message)
{
    if (sock < 0 || message == NULL)
    {
        errno = EBADF;
        return -1;
    }

    return send(sock, message, strlen(message), 0);
}

int receive_message_hub(int sock, char* buffer, size_t buffer_size)
{
    if (sock < 0 || buffer == NULL || buffer_size == 0)
    {
        errno = EBADF;
        return -1;
    }
    return recv(sock, buffer, buffer_size - 1, 0);
}
