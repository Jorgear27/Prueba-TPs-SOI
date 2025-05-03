#include "hub.h"
#include "listener.h"
#include "orders.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function prototypes
void print_usage(const char* program_name);
void display_menu(void);

void print_usage(const char* program_name)
{
    printf("Usage: %s <hub_id> <latitude> <longitude>\n", program_name);
}

void display_menu()
{
    printf("\n--- Hub Menu ---\n");
    printf("1. Create an order\n");
    printf("2. Cancel an order\n");
    printf("3. Query order status\n");
    printf("4. Disconnect\n");
    printf("Enter your choice: \n");
}

int main(int argc, char* argv[])
{
    // Check if the correct number of arguments is provided
    if (argc != 4)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments
    const char* hub_id = argv[1];
    int latitude = atoi(argv[2]);
    int longitude = atoi(argv[3]);

    // Step 1: Connect to the server
    int sock = connect_hub_to_server();

    // Step 2: Authenticate hub
    char* client_info = create_client_info_hub(hub_id, latitude, longitude);
    if (send_message_from_hub(sock, client_info) < 0)
    {
        perror("Failed to send client info");
        free(client_info);
        disconnect_hub_from_server(sock, hub_id);
        return EXIT_FAILURE;
    }
    free(client_info);

    // Step 3: Start the listener thread
    pthread_t listener_thread;
    if (pthread_create(&listener_thread, NULL, listen_for_updates, &sock) != 0)
    {
        perror("Failed to create listener thread");
        disconnect_hub_from_server(sock, hub_id);
        return EXIT_FAILURE;
    }

    // User interface loop
    int choice;
    while (1)
    {
        display_menu();
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            handle_create_order(sock, hub_id);
            break;
        case 2:
            handle_cancel_order(sock, hub_id);
            break;
        case 3:
            handle_query_order_status(sock, hub_id);
            break;
        case 4:
            disconnect_hub_from_server(sock, hub_id);

            pthread_cancel(listener_thread);     // Cancel the listener thread
            pthread_join(listener_thread, NULL); // Wait for the listener thread to finish
            return EXIT_SUCCESS;
        default:
            printf("Invalid choice. Please try again.\n");
        }
        usleep(100000); // Sleep for a short duration to allow printing to complete
    }

    return EXIT_SUCCESS;
}
