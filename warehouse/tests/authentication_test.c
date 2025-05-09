#include "authentication.h"
#include "unity.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WH_ID "W009"
#define WH_FAIL "M001"
#define LATITUDE 37
#define LONGITUDE -122

void setUp(void);
void tearDown(void);
void test_create_client_info(void);
void test_create_client_info_fail(void);
int main(void);

void setUp(void)
{
    // Setup code before each test
}

void tearDown(void)
{
    // Cleanup code after each test
}

// Test case for create_client_info_warehouse
void test_create_client_info()
{
    printf("Testing create_client_info_warehouse...\n");
    char* client_info = create_client_info_warehouse(WH_ID, LATITUDE, LONGITUDE);

    // Parse the JSON result
    cJSON* json_obj = cJSON_Parse(client_info);
    TEST_ASSERT_NOT_NULL(json_obj);

    // Validate the JSON fields
    cJSON* type = cJSON_GetObjectItem(json_obj, "type");
    TEST_ASSERT_TRUE(cJSON_IsString(type));
    TEST_ASSERT_EQUAL_STRING("client_info", type->valuestring);

    cJSON* timestamp = cJSON_GetObjectItem(json_obj, "timestamp");
    TEST_ASSERT_TRUE(cJSON_IsString(timestamp));
    TEST_ASSERT_TRUE(strlen(timestamp->valuestring) > 0);

    cJSON* warehouse_id_json = cJSON_GetObjectItem(json_obj, "warehouse_id");
    TEST_ASSERT_TRUE(cJSON_IsString(warehouse_id_json));
    TEST_ASSERT_EQUAL_STRING(WH_ID, warehouse_id_json->valuestring);

    cJSON* location = cJSON_GetObjectItem(json_obj, "location");
    TEST_ASSERT_TRUE(cJSON_IsObject(location));

    cJSON* latitude_json = cJSON_GetObjectItem(location, "latitude");
    TEST_ASSERT_TRUE(cJSON_IsNumber(latitude_json));
    TEST_ASSERT_EQUAL_INT(LATITUDE, latitude_json->valueint);

    cJSON* longitude_json = cJSON_GetObjectItem(location, "longitude");
    TEST_ASSERT_TRUE(cJSON_IsNumber(longitude_json));
    TEST_ASSERT_EQUAL_INT(LONGITUDE, longitude_json->valueint);

    printf("Client Info: %s\n", client_info);

    cJSON_Delete(json_obj);
    free(client_info);
}

// Test case for create_client_info_warehouse with invalid warehouse ID
void test_create_client_info_fail()
{
    printf("Testing create_client_info_warehouse with invalid warehouse ID...\n");
    char* client_info = create_client_info_warehouse(WH_FAIL, LATITUDE, LONGITUDE);

    // Parse the JSON result
    cJSON* json_obj = cJSON_Parse(client_info);
    TEST_ASSERT_NULL(json_obj); // Expecting NULL for invalid warehouse ID
    printf("Client info JSON for invalid ID: %s\n", client_info);

    // Clean up
    cJSON_Delete(json_obj);
    free(client_info);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_create_client_info);
    RUN_TEST(test_create_client_info_fail);
    return UNITY_END();
}
