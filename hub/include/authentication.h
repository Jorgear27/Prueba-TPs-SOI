/**
 * @file authentication.h
 * @brief Header file for the Authentication module.
 * @version 0.1
 * @date 2025-05-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "hub.h"
#include <cjson/cJSON.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Maximum length for the timestamp.
 */
#define MAX_TIMESTAMP_LENGTH 50

/**
 * @brief Generate the current timestamp in ISO 8601 format
 *
 */
void generate_timestamp_hub(char* buffer, size_t size);

/**
 * @brief Create a JSON string for client information.
 *
 * @param hub_id
 * @param latitude
 * @param longitude
 * @return client_info json string
 */
char* create_client_info_hub(const char* hub_id, int latitude, int longitude);

/**
 * @brief Validate the hub ID format.
 *
 * @param hub_id
 * @return true if valid, false otherwise
 */
bool isValidHubId(const char* hub_id);

#endif // AUTHENTICATION_H
