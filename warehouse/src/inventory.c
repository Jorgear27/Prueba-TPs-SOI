#include "inventory.h"

static Inventory inventory; // Global inventory instance

void initialize_inventory(int item_count)
{
    inventory.items = (InventoryItem*)malloc(sizeof(InventoryItem) * item_count);
    inventory.item_count = item_count;

    // Mutex for inventory access
    pthread_mutex_init(&inventory.lock, NULL);

    // Initialize all items with default values
    for (int i = 0; i < item_count; i++)
    {
        inventory.items[i].item_type = i;
        inventory.items[i].threshold = ITEM_THRESHOLD;
        inventory.items[i].stock_level = MAX_ITEM_QUANTITY;
    }
}

void update_inventory(int item_type, int quantity)
{
    pthread_mutex_lock(&inventory.lock);

    for (int i = 0; i < inventory.item_count; i++)
    {
        if (inventory.items[i].item_type == item_type)
        {
            inventory.items[i].stock_level += quantity;
            printf("[INFO] Updated inventory: Item %d, New Stock Level: %d\n", item_type,
                   inventory.items[i].stock_level);
            break;
        }
    }

    pthread_mutex_unlock(&inventory.lock);
}

void set_inventory(int item_type, int stock_level)
{
    pthread_mutex_lock(&inventory.lock);

    for (int i = 0; i < inventory.item_count; i++)
    {
        if (inventory.items[i].item_type == item_type)
        {
            inventory.items[i].stock_level = stock_level;
            printf("[INFO] Updated inventory: Item %d, New Stock Level: %d\n", item_type,
                   inventory.items[i].stock_level);
            break;
        }
    }

    pthread_mutex_unlock(&inventory.lock);
}

int check_restock_item(int item_type)
{
    pthread_mutex_lock(&inventory.lock);

    for (int i = 0; i < inventory.item_count; i++)
    {
        if (inventory.items[i].item_type == item_type)
        {
            if (inventory.items[i].stock_level < inventory.items[i].threshold)
            {
                // Trigger restock if below threshold
                printf("[WARNING] Item %d below threshold! Current stock_level: %d, Threshold: %d\n", item_type,
                       inventory.items[i].stock_level, inventory.items[i].threshold);
                // Manually restock
                inventory.items[i].stock_level = MAX_ITEM_QUANTITY;
                printf("[INFO] Restocked item %d. New Stock Level: %d\n", item_type, inventory.items[i].stock_level);
                pthread_mutex_unlock(&inventory.lock);
                return 1; // Restock triggered
            }
            break;
        }
    }

    pthread_mutex_unlock(&inventory.lock);
    return 0; // No restock needed
}

Inventory* get_inventory()
{
    return &inventory;
}

void print_inventory()
{
    pthread_mutex_lock(&inventory.lock);

    printf("[INFO] Current Inventory:\n");
    for (int i = 0; i < inventory.item_count; i++)
    {
        printf("Item Type: %d, Stock Level: %d\n", inventory.items[i].item_type, inventory.items[i].stock_level);
    }

    pthread_mutex_unlock(&inventory.lock);
}

void free_inventory()
{
    free(inventory.items);
    pthread_mutex_destroy(&inventory.lock);
}

void send_inventory_to_server(int sock, const char* wh_id)
{
    Inventory* inventory = get_inventory();

    // Create the root JSON object
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "inventory_update");
    cJSON_AddStringToObject(root, "user_id", wh_id);

    // Generate a timestamp
    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_wh(timestamp, sizeof(timestamp));
    cJSON_AddStringToObject(root, "timestamp", timestamp);

    // Create the inventory array
    cJSON* inventory_array = cJSON_CreateArray();

    pthread_mutex_lock(&inventory->lock); // Lock the inventory for thread-safe access

    for (int i = 0; i < inventory->item_count; i++)
    {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "item_type", inventory->items[i].item_type);
        cJSON_AddNumberToObject(item, "stock_level", inventory->items[i].stock_level);
        cJSON_AddNumberToObject(item, "threshold", inventory->items[i].threshold);
        cJSON_AddItemToArray(inventory_array, item);
    }

    pthread_mutex_unlock(&inventory->lock); // Unlock the inventory

    cJSON_AddItemToObject(root, "inventory", inventory_array);

    // Convert the JSON object to a string
    char* json_string = cJSON_PrintUnformatted(root);

    // Send the JSON string to the server
    if (send(sock, json_string, strlen(json_string), 0) < 0)
    {
        perror("[ERROR] Failed to send inventory to server");
    }
    else
    {
        printf("[INFO] Inventory sent to server: %s\n", json_string);
    }

    // Clean up
    free(json_string);
    cJSON_Delete(root);
}

void send_restock_to_server(int sock, const char* wh_id, int item_type)
{
    // Create the root JSON object
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "restock_notice");
    cJSON_AddStringToObject(root, "user_id", wh_id);

    // Generate a timestamp
    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_wh(timestamp, sizeof(timestamp));
    cJSON_AddStringToObject(root, "timestamp", timestamp);

    // Add item type and stock_level to the JSON object as an int
    cJSON_AddNumberToObject(root, "item_type", item_type);
    cJSON_AddNumberToObject(root, "stock_level", MAX_ITEM_QUANTITY);

    // Convert the JSON object to a string
    char* json_string = cJSON_PrintUnformatted(root);

    // Send the JSON string to the server
    if (send(sock, json_string, strlen(json_string), 0) < 0)
    {
        perror("[ERROR] Failed to send restock notice to server");
    }
    else
    {
        printf("[INFO] Restock notice sent to server: %s\n", json_string);
    }

    // Clean up
    free(json_string);
    cJSON_Delete(root);
}

void* periodic_inventory_update(void* args)
{
    // Cast the argument to ThreadArgs*
    ThreadArgs* thread_args = (ThreadArgs*)args;

    // Extract the socket and warehouse ID
    int sock = thread_args->sock;
    const char* wh_id = thread_args->wh_id;

    while (true)
    {
        // Wait before the next update
        sleep(SLEEP_TIME);

        // Send the inventory to the server
        send_inventory_to_server(sock, wh_id);
    }

    return NULL;
}
