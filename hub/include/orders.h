/**
 * @file orders.h
 * @brief Header file for the Orders module.
 * @version 0.1
 * @date 2025-04-08
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ORDERS_H
#define ORDERS_H

#include <stddef.h>

/**
 * @brief Socket buffer size for receiving messages.
 */
#define SOCKET_BUFFER_SIZE 1024
/**
 * @brief Maximum length for the order ID.
 */
#define MAX_ORDER_ID_LENGTH 50
/**
 * @brief Maximum length for the timestamp.
 */
#define MAX_TIMESTAMP_LENGTH 50

/**
 * @brief Generate a timestamp in ISO 8601 format.
 *
 */
void generate_timestamp_hub(char* buffer, size_t size);

/**
 * @brief Handle the creation of an order.
 *
 * @param sock
 * @param hub_id
 */
void handle_create_order(int sock, const char* hub_id);

/**
 * @brief Handle the cancellation of an order.
 *
 * @param sock
 * @param hub_id
 */
void handle_cancel_order(int sock, const char* hub_id);

/**
 * @brief Handle the query of an order status.
 *
 * @param sock
 * @param hub_id
 */
void handle_query_order_status(int sock, const char* hub_id);

/**
 * @brief Create a JSON string for client information.
 *
 * @param hub_id ID of the hub.
 * @param latitude Latitude of the client.
 * @param longitude Longitude of the client.
 * @return char* JSON string containing client information.
 */
char* create_client_info_hub(const char* hub_id, int latitude, int longitude);

/**
 * @brief Create a JSON string for an order request.
 *
 * @param order_id
 * @param hub_id
 * @param timestamp
 * @param items_needed
 * @return order_request json string
 */
char* create_order_request(const char* order_id, const char* hub_id, const char* timestamp, const char* items_needed);

#endif // ORDERS_H
