#include "authentication.h"
#include "inventory.h"
#include "unity.h"

// Test authentication functions
extern void test_create_client_info(void);
extern void test_create_client_info_fail(void);

// Test inventory functions

extern void test_initialize_inventory(void);
extern void test_update_inventory_addition(void);
extern void test_update_inventory_remove(void);
extern void test_set_inventory(void);
extern void test_check_restock_item_no_restock(void);
extern void test_check_restock_item_with_restock(void);
extern void test_send_inventory_to_server(void);

void setUp(void);
void tearDown(void);

void setUp(void)
{
    // setUp
}

void tearDown(void)
{
    // TearDown
}

int main(void)
{
    UNITY_BEGIN();

    // Run tests from inventory_test.c
    printf("[INFO] Running inventory tests...\n");
    RUN_TEST(test_initialize_inventory);
    RUN_TEST(test_update_inventory_addition);
    RUN_TEST(test_update_inventory_remove);
    RUN_TEST(test_set_inventory);
    RUN_TEST(test_check_restock_item_no_restock);
    RUN_TEST(test_check_restock_item_with_restock);
    RUN_TEST(test_send_inventory_to_server);
    printf("[INFO] Inventory tests completed.\n");
    printf("/n");

    // Run tests from authentication_test.c
    printf("[INFO] Running authentication tests...\n");
    RUN_TEST(test_create_client_info);
    RUN_TEST(test_create_client_info_fail);
    printf("[INFO] Authentication tests completed.\n");
    printf("/n");

    return UNITY_END();
}
