#include "authentication.h"

// Helper function to generate the current timestamp in ISO 8601 format
void generate_timestamp_hub(char* buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm* t = gmtime(&now);
    strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", t);
}

char* create_client_info_hub(const char* hub_id, int latitude, int longitude)
{
    // Validate the hub_id
    if (!isValidHubId(hub_id))
    {
        return NULL; // Invalid hub_id
    }

    cJSON* json_obj = cJSON_CreateObject();

    // Generate the current timestamp
    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_hub(timestamp, sizeof(timestamp));

    cJSON_AddStringToObject(json_obj, "type", "client_info");
    cJSON_AddStringToObject(json_obj, "timestamp", timestamp);
    cJSON_AddStringToObject(json_obj, "hub_id", hub_id);

    cJSON* location = cJSON_CreateObject();
    cJSON_AddNumberToObject(location, "latitude", latitude);
    cJSON_AddNumberToObject(location, "longitude", longitude);
    cJSON_AddItemToObject(json_obj, "location", location);

    char* result = cJSON_PrintUnformatted(json_obj);

    cJSON_Delete(json_obj); // Free JSON object
    return result;
}

bool isValidHubId(const char* hub_id)
{
    // Verify that the warehouse ID is not NULL
    if (strlen(hub_id) < 2)
    {
        return false;
    }

    // Verify that the first character is 'H'
    if (hub_id[0] != 'H')
    {
        return false;
    }

    // Verify that the rest of the string contains only digits
    for (size_t i = 1; i < strlen(hub_id); ++i)
    {
        if (!isdigit(hub_id[i]))
        {
            return false;
        }
    }

    return true;
}
