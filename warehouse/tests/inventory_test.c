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
#define TEST_SOCKET 42

static int mock_send_message_result = 0; // Global variable to simulate send_message_from_wh result
char mock_message_buffer[1024] = {0};    // Buffer to store the sent message

void setUp(void);
void tearDown(void);
void test_initialize_inventory(void);
void test_update_inventory_addition(void);
void test_update_inventory_remove(void);
void test_set_inventory(void);
void test_check_restock_item_no_restock(void);
void test_check_restock_item_with_restock(void);
void test_send_inventory_to_server(void);
void test_send_restock_to_server(void);
int main(void);

// Mock definitions
#ifdef TESTING
// Mock prototypes
int send_message_from_wh(int sock, const char* message);
void generate_timestamp_wh(char* buffer, size_t size);

int send_message_from_wh(int sock, const char* message)
{
    (void)sock;
    strncpy(mock_message_buffer, message, sizeof(mock_message_buffer) - 1);
    printf("[MOCK] Sending message: %s\n", message);
    return mock_send_message_result; // Simulate success or failure
}

void generate_timestamp_wh(char* buffer, size_t size)
{
    // Simulate a fixed timestamp for testing
    const char* mock_timestamp = "2025-01-01T00:00:00Z";
    strncpy(buffer, mock_timestamp, size - 1);
    buffer[size - 1] = '\0';
}

#endif

void setUp(void)
{
    initialize_inventory(TEST_ITEM_COUNT);
}

void tearDown(void)
{
    free_inventory();
}

// Test initialization of inventory
void test_initialize_inventory(void)
{
    printf("Testing initialize_inventory...\n");
    Inventory* inv = get_inventory();
    TEST_ASSERT_NOT_NULL(inv->items);
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_TYPES, inv->item_count);

    for (int i = 0; i < MAX_ITEM_TYPES; i++)
    {
        TEST_ASSERT_EQUAL_INT(i, inv->items[i].item_type);
        TEST_ASSERT_EQUAL_INT(ITEM_THRESHOLD, inv->items[i].threshold);
        TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY, inv->items[i].stock_level);
    }
}

// Test updating inventory by adding items
void test_update_inventory_addition(void)
{
    printf("Testing update_inventory to add items...\n");
    update_inventory(TEST_ITEM_TYPE, TEST_QUANTITY_POSITIVE);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY + TEST_QUANTITY_POSITIVE, inv->items[TEST_ITEM_TYPE].stock_level);
}

// Test updating inventory by removing items
void test_update_inventory_remove(void)
{
    printf("Testing update_inventory to remove items...\n");
    update_inventory(TEST_ITEM_TYPE, TEST_QUANTITY_NEGATIVE);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY + TEST_QUANTITY_NEGATIVE, inv->items[TEST_ITEM_TYPE].stock_level);
}

// Test setting inventory directly
void test_set_inventory(void)
{
    printf("Testing set_inventory forced...\n");
    set_inventory(TEST_ITEM_TYPE, TEST_STOCK_LEVEL);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(TEST_STOCK_LEVEL, inv->items[TEST_ITEM_TYPE].stock_level);
}

// Test checking if restock is needed, without triggering restock
void test_check_restock_item_no_restock(void)
{
    printf("Testing check_restock_item with no restock needed...\n");
    set_inventory(TEST_ITEM_TYPE, ITEM_THRESHOLD + 1);
    int restock_triggered = check_restock_item(TEST_ITEM_TYPE);
    TEST_ASSERT_EQUAL_INT(0, restock_triggered);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT((ITEM_THRESHOLD + 1), inv->items[TEST_ITEM_TYPE].stock_level);
}

// Test checking if restock is needed, triggering restock
void test_check_restock_item_with_restock(void)
{
    printf("Testing check_restock_item with restock needed...\n");
    set_inventory(TEST_ITEM_TYPE, ITEM_THRESHOLD - 1);
    int restock_triggered = check_restock_item(TEST_ITEM_TYPE);
    TEST_ASSERT_EQUAL_INT(1, restock_triggered);

    Inventory* inv = get_inventory();
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY, inv->items[TEST_ITEM_TYPE].stock_level);
}

// Test sending inventory to server
void test_send_inventory_to_server(void)
{
    printf("Testing send_inventory_to_server...\n");
    // Set up the inventory
    set_inventory(TEST_ITEM_TYPE, TEST_STOCK_LEVEL);
    send_inventory_to_server(TEST_SOCKET, TEST_WAREHOUSE_ID);

    // Check if the message was sent
    TEST_ASSERT_NOT_EQUAL(0, strlen(mock_message_buffer));
    cJSON* root = cJSON_Parse(mock_message_buffer);
    TEST_ASSERT_NOT_NULL(root);

    // Validate JSON structure
    cJSON* type = cJSON_GetObjectItem(root, "type");
    TEST_ASSERT_NOT_NULL(type);
    TEST_ASSERT_EQUAL_STRING("inventory_update", type->valuestring);

    cJSON* user_id = cJSON_GetObjectItem(root, "user_id");
    TEST_ASSERT_NOT_NULL(user_id);
    TEST_ASSERT_EQUAL_STRING(TEST_WAREHOUSE_ID, user_id->valuestring);

    // Validate inventory array
    cJSON* inventory_array = cJSON_GetObjectItem(root, "inventory");
    TEST_ASSERT_NOT_NULL(inventory_array);
    TEST_ASSERT(cJSON_IsArray(inventory_array));

    // Ensure TEST_ITEM_TYPE is within bounds
    int inventory_size = cJSON_GetArraySize(inventory_array);
    TEST_ASSERT(TEST_ITEM_TYPE >= 0 && TEST_ITEM_TYPE < inventory_size);

    // Access the item at the position corresponding to TEST_ITEM_TYPE
    cJSON* item = cJSON_GetArrayItem(inventory_array, TEST_ITEM_TYPE);
    TEST_ASSERT_NOT_NULL(item);

    // Validate the item_type and quantity
    cJSON* item_type = cJSON_GetObjectItem(item, "item_type");
    TEST_ASSERT_NOT_NULL(item_type);
    TEST_ASSERT_EQUAL_INT(TEST_ITEM_TYPE, item_type->valueint);

    cJSON* quantity = cJSON_GetObjectItem(item, "stock_level");
    TEST_ASSERT_NOT_NULL(quantity);
    TEST_ASSERT_EQUAL_INT(TEST_STOCK_LEVEL, quantity->valueint);

    // Clean up
    cJSON_Delete(root);
}

// Test sending restock notice to server
void test_send_restock_to_server(void)
{
    printf("Testing send_restock_to_server...\n");

    // Send a restock notice
    send_restock_to_server(TEST_SOCKET, TEST_WAREHOUSE_ID, TEST_ITEM_TYPE);

    // Check if the message was sent
    TEST_ASSERT_NOT_EQUAL(0, strlen(mock_message_buffer));
    cJSON* root = cJSON_Parse(mock_message_buffer);
    TEST_ASSERT_NOT_NULL(root);

    // Validate JSON structure
    cJSON* type = cJSON_GetObjectItem(root, "type");
    TEST_ASSERT_NOT_NULL(type);
    TEST_ASSERT_EQUAL_STRING("restock_notice", type->valuestring);

    cJSON* item_type = cJSON_GetObjectItem(root, "item_type");
    TEST_ASSERT_NOT_NULL(item_type);
    TEST_ASSERT_EQUAL_INT(TEST_ITEM_TYPE, item_type->valueint);

    cJSON* quantity = cJSON_GetObjectItem(root, "stock_level");
    TEST_ASSERT_NOT_NULL(quantity);
    TEST_ASSERT_EQUAL_INT(MAX_ITEM_QUANTITY, quantity->valueint);

    // Clean up
    cJSON_Delete(root);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initialize_inventory);
    RUN_TEST(test_update_inventory_addition);
    RUN_TEST(test_update_inventory_remove);
    RUN_TEST(test_set_inventory);
    RUN_TEST(test_check_restock_item_no_restock);
    RUN_TEST(test_check_restock_item_with_restock);
    RUN_TEST(test_send_inventory_to_server);
    RUN_TEST(test_send_restock_to_server);
    return UNITY_END();
}
