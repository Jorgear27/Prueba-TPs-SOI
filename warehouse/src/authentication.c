#include "authentication.h"

void generate_timestamp_wh(char* buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm* t = gmtime(&now);
    strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", t);
}

char* create_client_info_warehouse(const char* wh_id, int latitude, int longitude)
{

    // Validar el formato de wh_id
    if (wh_id == NULL || wh_id[0] != 'W')
    {
        return NULL; // Retornar NULL si el formato es inválido
        printf("Invalid warehouse ID format\n");
    }

    // Create a JSON object for client information
    cJSON* json_obj = cJSON_CreateObject();

    // Generate the current timestamp
    char timestamp[MAX_TIMESTAMP_LENGTH];
    generate_timestamp_wh(timestamp, sizeof(timestamp));

    cJSON_AddStringToObject(json_obj, "type", "client_info");
    cJSON_AddStringToObject(json_obj, "timestamp", timestamp);
    cJSON_AddStringToObject(json_obj, "warehouse_id", wh_id);

    cJSON* location = cJSON_CreateObject();
    cJSON_AddNumberToObject(location, "latitude", latitude);
    cJSON_AddNumberToObject(location, "longitude", longitude);
    cJSON_AddItemToObject(json_obj, "location", location);

    char* result = cJSON_PrintUnformatted(json_obj);

    // Clean up the JSON object
    cJSON_Delete(json_obj);

    return result;
}
