#include "orders.h"
#include "unity.h"
#include <cjson/cJSON.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function prototypes
void setUp(void);
void tearDown(void);
void test_handle_create_order_valid(void);
void test_handle_create_order_invalid_item_type(void);
void test_handle_cancel_order(void);
void test_handle_query_order_status(void);
void test_create_client_info_hub(void);
void test_create_order_request(void);
int main(void);

// Mock definitions
#ifdef TESTING
// Mock prototypes
int send_message_from_hub(int sock, const char* message);
int receive_message_hub(int sock, char* buffer, size_t buffer_size);
void generate_timestamp_hub(char* buffer, size_t size);

static int mock_send_message_result = 0;
static const char* mock_received_message = NULL;
static const char* mock_scanf_inputs[10]; // Array to hold mock inputs
static int mock_scanf_index = 0;          // Index to track the current input

int send_message_from_hub(int sock, const char* message)
{
    (void)sock;
    printf("[MOCK] Sending message: %s\n", message);
    return mock_send_message_result; // Simulate success or failure
}

int receive_message_hub(int sock, char* buffer, size_t buffer_size)
{
    (void)sock;
    if (mock_received_message == NULL)
    {
        return -1; // Simulate failure
    }
    strncpy(buffer, mock_received_message, buffer_size);
    return strlen(mock_received_message); // Simulate receiving a message
}

void generate_timestamp_hub(char* buffer, size_t size)
{
    // Simulate a fixed timestamp for testing
    const char* mock_timestamp = "2025-01-01T00:00:00Z";
    strncpy(buffer, mock_timestamp, size - 1);
    buffer[size - 1] = '\0';
}

int scanf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    if (mock_scanf_inputs[mock_scanf_index] != NULL)
    {
        if (strstr(format, "%d") != NULL)
        {
            *(va_arg(args, int*)) = atoi(mock_scanf_inputs[mock_scanf_index]); // Simulate integer input
        }
        else if (strstr(format, "%s") != NULL)
        {
            strncpy(va_arg(args, char*), mock_scanf_inputs[mock_scanf_index],
                    MAX_ORDER_ID_LENGTH); // Simulate string input
        }
        mock_scanf_index++; // Move to the next input
    }

    va_end(args);
    return 1; // Simulate successful input
}
#endif

void setUp()
{
    // Setup code before each test
    mock_send_message_result = 0;
    mock_received_message = NULL;
    mock_scanf_index = 0;
    memset(mock_scanf_inputs, 0, sizeof(mock_scanf_inputs)); // Clear the input array
}

void tearDown()
{
    // Cleanup code after each test
}

// Test handle_create_order
void test_handle_create_order_valid()
{
    printf("Testing handle_create_order with valid inputs...\n");
    // Simulate inputs for item type and quantity
    mock_scanf_inputs[0] = "0";  // Item type: Medicines
    mock_scanf_inputs[1] = "10"; // Quantity: 10
    mock_scanf_inputs[2] = "-1"; // Finish adding items
    mock_scanf_index = 0;        // Reset the input index

    int sock = 42;
    handle_create_order(sock, "hub_1");

    // Verify behavior with mock logs
    // Expect no errors and successful order creation
}

void test_handle_create_order_invalid_item_type()
{
    printf("Testing handle_create_order with invalid item type...\n");
    // Simulate invalid item type input
    mock_scanf_inputs[0] = "3";  // Invalid item type
    mock_scanf_inputs[1] = "10"; // Quantity: 10
    mock_scanf_inputs[2] = "-1"; // Finish adding items
    int sock = 42;
    handle_create_order(sock, "hub_1");
    // Verify behavior with mock logs
    // Expect an error message indicating invalid item type
    // and no order creation
}

void test_handle_cancel_order()
{
    printf("Testing handle_cancel_order...\n");
    // Simulate entering an order ID
    mock_scanf_inputs[0] = "order_123"; // Order ID to cancel
    mock_scanf_index = 0;               // Reset the input index
    int sock = 42;
    handle_cancel_order(sock, "hub_1");
    // Verify behavior with mock logs
    // Expect no errors and successful order cancellation
}

void test_handle_query_order_status()
{
    printf("Testing handle_query_order_status...\n");
    // Simulate entering an order ID
    mock_scanf_inputs[0] = "order_123"; // Order ID to query
    mock_scanf_index = 0;               // Reset the input index
    // Simulate receiving a response from the server
    mock_received_message = "{\"type\":\"order_status\",\"timestamp\":\"2025-05-02T12:00:00Z\",\"order_id\":\"12345\","
                            "\"status\":\"shipped\",\"items_shipped\":[{\"item_type\":1,\"quantity\":10}]}";
    int sock = 42;
    handle_query_order_status(sock, "hub_1");
    // Verify behavior with mock logs
    // Expect no errors and successful order status query
}

void test_create_order_request()
{
    printf("Testing create_order_request...\n");
    const char* items_needed = "[{\"item_type\":1,\"quantity\":10}]";
    char* order_request = create_order_request("order_123", "hub_1", "2025-05-02T12:00:00Z", items_needed);
    TEST_ASSERT_NOT_NULL(order_request);
    printf("Order Request: %s\n", order_request);
    free(order_request);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_handle_create_order_valid);
    RUN_TEST(test_handle_create_order_invalid_item_type);
    RUN_TEST(test_handle_cancel_order);
    RUN_TEST(test_handle_query_order_status);
    RUN_TEST(test_create_order_request);

    return UNITY_END();
}
