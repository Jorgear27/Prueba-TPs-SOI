/**
 * @file inventory.h
 * @brief Header file for the Authentication module.
 * @version 0.1
 * @date 2025-04-29
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "warehouse.h"
#include <cjson/cJSON.h>
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
 * @brief Generate a timestamp in ISO 8601 format.
 *
 */
void generate_timestamp_wh(char* buffer, size_t size);

/**
 * @brief Create a JSON string for client information.
 *
 * @param wh_id
 * @param latitude
 * @param longitude
 * @return client_info json string
 */
char* create_client_info_warehouse(const char* wh_id, int latitude, int longitude);

#endif // AUTHENTICATION_H
