#include "orders.h"
#include "inventory.h"

#include "inventory.h"
#include "orders.h"

int send_orderdispatch_to_server(int sock, const char* order_id, const char* items_needed)
{
    cJSON* response = cJSON_CreateObject();

    cJSON_AddStringToObject(response, "type", "order_dispatch");
    cJSON_AddStringToObject(response, "order_id", order_id);
    cJSON_AddStringToObject(response, "status", "Shipped");

    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_wh(timestamp, sizeof(timestamp));
    cJSON_AddStringToObject(response, "timestamp", timestamp);

    cJSON* itemsArray = cJSON_Parse(items_needed);
    if (itemsArray == NULL || !cJSON_IsArray(itemsArray))
    {
        printf("[ERROR] Invalid items_needed format. Failed to parse JSON.\n");
        cJSON_Delete(response);
        return -1;
    }

    cJSON_AddItemToObject(response, "items_shipped", itemsArray);

    char* json_string = cJSON_PrintUnformatted(response);

    if (send_message_from_wh(sock, json_string) < 0)
    {
        perror("[ERROR] Failed to send inventory to server");
        free(json_string);
        cJSON_Delete(response);
        return -1;
    }
    else
    {
        printf("[INFO] Inventory sent to server: %s\n", json_string);
    }

    free(json_string);
    cJSON_Delete(response);

    return 0;
}
