
#include "authentication.h"
#include "inventory.h"
#include "unity.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#define TEST_ITEM_COUNT 3
#define TEST_ITEM_TYPE 2
#define TEST_QUANTITY_POSITIVE 10
#define TEST_QUANTITY_NEGATIVE -10
#define TEST_STOCK_LEVEL 50
#define TEST_WAREHOUSE_ID "W005"
#define TEST_PORT 8080
#define TEST_IP "127.0.0.1"

int mock_server_socket;

void test_initialize_inventory(void);
void test_update_inventory_addition(void);
void test_update_inventory_remove(void);
void test_set_inventory(void);
void test_check_restock_item_no_restock(void);
void test_check_restock_item_with_restock(void);
void test_send_inventory_to_server(void);

void test_initialize_inventory(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
    Inventory* inv = get_inventory();
    TEST_ASSERT_NOT_NULL(inv->items);
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_TYPES, inv->item_count);

    for (int i = 0; i < MAX_ITEM_TYPES; i++)
    {
        TEST_ASSERT_EQUAL_INT(i, inv->items[i].item_type);
        TEST_ASSERT_EQUAL_INT(ITEM_THRESHOLD, inv->items[i].threshold);
        TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY, inv->items[i].stock_level);
    }

    free_inventory();
}

void test_update_inventory_addition(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
    update_inventory(TEST_ITEM_TYPE, TEST_QUANTITY_POSITIVE);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY + TEST_QUANTITY_POSITIVE, inv->items[TEST_ITEM_TYPE].stock_level);

    free_inventory();
}

void test_update_inventory_remove(void) // Initialize the inventory in the setup, verify it here
{
    initialize_inventory(TEST_ITEM_COUNT);
    update_inventory(TEST_ITEM_TYPE, TEST_QUANTITY_NEGATIVE);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY + TEST_QUANTITY_NEGATIVE, inv->items[TEST_ITEM_TYPE].stock_level);

    free_inventory();
}

void test_set_inventory(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
    set_inventory(TEST_ITEM_TYPE, TEST_STOCK_LEVEL);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(TEST_STOCK_LEVEL, inv->items[TEST_ITEM_TYPE].stock_level);

    free_inventory();
}

void test_check_restock_item_no_restock(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
    set_inventory(TEST_ITEM_TYPE, ITEM_THRESHOLD + 1);
    int restock_triggered = check_restock_item(TEST_ITEM_TYPE);
    TEST_ASSERT_EQUAL_INT(0, restock_triggered);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT((ITEM_THRESHOLD + 1), inv->items[TEST_ITEM_TYPE].stock_level);

    free_inventory();
}

void test_check_restock_item_with_restock(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
    set_inventory(TEST_ITEM_TYPE, ITEM_THRESHOLD - 1);
    int restock_triggered = check_restock_item(TEST_ITEM_TYPE);
    TEST_ASSERT_EQUAL_INT(1, restock_triggered);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY, inv->items[TEST_ITEM_TYPE].stock_level);

    free_inventory();
}

void test_send_inventory_to_server(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
    // Create a mock server socket
    mock_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(TEST_PORT),
        .sin_addr.s_addr = inet_addr(TEST_IP),
    };
    bind(mock_server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(mock_server_socket, 1);

    // Create a client socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Accept the connection on the server side
    int server_socket = accept(mock_server_socket, NULL, NULL);

    // Send inventory to server
    send_inventory_to_server(client_socket, TEST_WAREHOUSE_ID);

    // Read the data sent to the server
    char buffer[1024] = {0};
    read(server_socket, buffer, sizeof(buffer));

    // Print the received data for debugging
    printf("[DEBUG] Data sent to server: %s\n", buffer);

    // Validate the JSON structure
    cJSON* root = cJSON_Parse(buffer);
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL_STRING("inventory_update", cJSON_GetObjectItem(root, "type")->valuestring);
    TEST_ASSERT_EQUAL_STRING(TEST_WAREHOUSE_ID, cJSON_GetObjectItem(root, "user_id")->valuestring);

    cJSON* inventory_array = cJSON_GetObjectItem(root, "inventory");
    TEST_ASSERT_NOT_NULL(inventory_array);
    TEST_ASSERT_EQUAL_INT(TEST_ITEM_COUNT, cJSON_GetArraySize(inventory_array));

    for (int i = 0; i < TEST_ITEM_COUNT; i++)
    {
        cJSON* item = cJSON_GetArrayItem(inventory_array, i);
        TEST_ASSERT_NOT_NULL(item);
        TEST_ASSERT_EQUAL_INT(i, cJSON_GetObjectItem(item, "item_type")->valueint);
        TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY, cJSON_GetObjectItem(item, "stock_level")->valueint);
        TEST_ASSERT_EQUAL_INT(ITEM_THRESHOLD, cJSON_GetObjectItem(item, "threshold")->valueint);
    }

    // Clean up
    cJSON_Delete(root);
    close(client_socket);
    close(server_socket);
    close(mock_server_socket);
    free_inventory();
}
