#include "database.hpp"

// Constructor
Database::Database(const std::string& connectionString)
{
    // Connect to the database using the connection string
    conn = PQconnectdb(connectionString.c_str());
    // Check if the connection was successful
    if (PQstatus(conn) != CONNECTION_OK)
    {
        // Log the error message
        Logger::getInstance().log("Database",
                                  "[ERROR] Connection to database failed: " + std::string(PQerrorMessage(conn)));
        std::cerr << "[ERROR] Connection to database failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn); // If fails, free the failed connection
        conn = nullptr; // Ensure the pointer does not point to an invalid connection
    }
    else
    {
        // If the connection is successful log the success message
        Logger::getInstance().log("Database", "[INFO] Connected to paranoid_db database successfully.");
        std::cout << "[INFO] Connected to paranoid_db database successfully. \n";
    }
}

// Destructor
Database::~Database()
{
    disconnect(); // Call the method to close the connection
}

// Check if the connection is active
bool Database::is_connected()
{
    // Return true if the pointer is not null and the connection is in OK status
    return conn != nullptr && PQstatus(conn) == CONNECTION_OK;
}

// Close the connection to the database
void Database::disconnect()
{
    // Check if there is an active connection
    if (conn)
    {
        PQfinish(conn); // Close the connection and release resources
        conn = nullptr; // Ensure the pointer does not point to a closed connection
    }
}

// Singleton: return the instance of the Database class
Database& Database::getInstance(const std::string& connectionString)
{
    static Database instance(connectionString);
    return instance;
}

PGconn* Database::getConnection(const std::string& connectionString)
{
    if (is_connected())
    {
        return conn; // Return the existing connection if it's active
    }

    // Attempt to reconnect if the connection is lost
    // Log the error message
    Logger::getInstance().log("Database", "[ERROR] Database connection lost. Attempting to reconnect...");
    std::cerr << "[ERROR] Database connection lost. Attempting to reconnect...\n";
    disconnect(); // Close the existing connection if any

    int retryCount = 0;
    const int maxRetries = 3; // Maximum number of reconnection attempts

    while (retryCount < maxRetries)
    {
        // Attempt to reconnect
        conn = PQconnectdb(connectionString.c_str());
        if (PQstatus(conn) == CONNECTION_OK)
        {
            // Log the success message
            Logger::getInstance().log("Database", "[INFO] Reconnected to database successfully.");
            std::cout << "[INFO] Reconnected to database successfully.\n";
            return conn; // Return the new connection if successful
        }
        else
        {
            // Log the error message
            Logger::getInstance().log("Database", "[ERROR] Reconnection attempt " + std::to_string(retryCount + 1) +
                                                      " failed: " + std::string(PQerrorMessage(conn)));
            std::cerr << "[ERROR] Reconnection attempt " << retryCount + 1 << " failed: " << PQerrorMessage(conn)
                      << "\n";
            retryCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait before retrying
        }
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        // Log the error message
        Logger::getInstance().log("Database",
                                  "[ERROR] Reconnection to database failed: " + std::string(PQerrorMessage(conn)));
        std::cerr << "[ERROR] Failed to reconnect: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        conn = nullptr;
    }

    // Return the connection object
    return conn;
}

bool Database::insertOrUpdateOrder(const std::string& orderId, const std::string& hubId, int itemType, int quantity)
{
    try
    {
        std::string query =
            "INSERT INTO orders (order_id, user_id, item_type, quantity, status) VALUES ('" + orderId + "', '" + hubId +
            "', " + std::to_string(itemType) + ", " + std::to_string(quantity) +
            ", 'Pending') ON CONFLICT (order_id, item_type) DO UPDATE SET quantity = " + std::to_string(quantity) +
            ", status = 'Pending';";

        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            Logger::getInstance().log("Database",
                                      "[ERROR] Error inserting/updating order: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return false;
        }
        PQclear(res);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

std::string Database::getOrderStatus(const std::string& orderId)
{
    try
    {
        std::string query = "SELECT status FROM orders WHERE order_id = '" + orderId + "';";
        PGresult* res = PQexec(conn, query.c_str());

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            Logger::getInstance().log("Database",
                                      "[ERROR] Error fetching order status: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return "";
        }

        std::string status = PQntuples(res) > 0 ? PQgetvalue(res, 0, 0) : "";
        PQclear(res);
        return status;
    }
    catch (const std::exception& e)
    {
        return "";
    }
}

bool Database::updateOrderStatus(const std::string& orderId, const std::string& newStatus)
{
    try
    {
        std::string query = "UPDATE orders SET status = '" + newStatus + "' WHERE order_id = '" + orderId + "';";
        PGresult* res = PQexec(conn, query.c_str());

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            Logger::getInstance().log("Database",
                                      "[ERROR] Error updating order status: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return false;
        }
        PQclear(res);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

nlohmann::json Database::getOrderDetails(const std::string& orderId)
{
    try
    {
        std::string query = R"(
            SELECT
                o.order_id,
                o.user_id,
                o.status,
                json_agg(json_build_object('item_type', o.item_type, 'quantity', o.quantity)) AS items_needed
            FROM orders o
            WHERE o.order_id = $1
            GROUP BY o.order_id, o.user_id, o.status;
        )";

        const char* paramValues[1] = {orderId.c_str()};
        PGresult* res = PQexecParams(conn, query.c_str(), 1, nullptr, paramValues, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            Logger::getInstance().log("Database",
                                      "[ERROR] Error fetching order details: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return nlohmann::json{{"status", "error"}, {"message", "Error fetching order details"}};
        }

        if (PQntuples(res) == 0)
        {
            PQclear(res);
            return nlohmann::json{{"status", "error"}, {"message", "No orders found with the specified ID"}};
        }

        nlohmann::json result = nlohmann::json::object();
        result["order_id"] = PQgetvalue(res, 0, 0);
        result["user_id"] = PQgetvalue(res, 0, 1);
        result["status"] = PQgetvalue(res, 0, 2);
        result["items_needed"] = nlohmann::json::parse(PQgetvalue(res, 0, 3));

        PQclear(res);
        return result;
    }
    catch (const std::exception& e)
    {
        return nlohmann::json{{"status", "error"}, {"message", "Failed to process get order details"}};
    }
}

std::vector<std::string> Database::getApprovedOrders()
{
    std::vector<std::string> approvedOrders;

    try
    {
        std::string query = "SELECT DISTINCT order_id FROM orders WHERE status = 'Approved';";
        PGresult* res = PQexec(conn, query.c_str());

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            Logger::getInstance().log("Database",
                                      "[ERROR] Error fetching approved orders: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return approvedOrders;
        }

        int numRows = PQntuples(res);
        for (int i = 0; i < numRows; ++i)
        {
            approvedOrders.push_back(PQgetvalue(res, i, 0));
        }

        PQclear(res);
    }
    catch (const std::exception& e)
    {
    }

    return approvedOrders;
}

bool Database::insertOrUpdateUser(const std::string& userId, double latitude, double longitude)
{
    try
    {
        std::string query = "INSERT INTO users (user_id, latitude, longitude, is_online) "
                            "VALUES ($1, $2, $3, TRUE) "
                            "ON CONFLICT (user_id) DO UPDATE "
                            "SET latitude = EXCLUDED.latitude, longitude = EXCLUDED.longitude, is_online = TRUE;";

        const char* paramValues[3] = {userId.c_str(), std::to_string(latitude).c_str(),
                                      std::to_string(longitude).c_str()};
        PGresult* res = PQexecParams(conn, query.c_str(), 3, nullptr, paramValues, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << "[ERROR] Error inserting/updating user: " << PQerrorMessage(conn) << "\n";
            Logger::getInstance().log("Database",
                                      "[ERROR] Error inserting/updating user: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return false;
        }

        PQclear(res);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool Database::updateUserOnlineStatus(const std::string& userId, bool isOnline)
{
    try
    {
        std::string query =
            "UPDATE users SET is_online = " + std::string(isOnline ? "TRUE" : "FALSE") + " WHERE user_id = $1;";
        const char* paramValues[1] = {userId.c_str()};
        PGresult* res = PQexecParams(conn, query.c_str(), 1, nullptr, paramValues, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            Logger::getInstance().log("Database", "[ERROR] Error updating user online status: " +
                                                      std::string(PQerrorMessage(conn)));
            PQclear(res);
            return false;
        }

        PQclear(res);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool Database::insertOrUpdateInventory(const std::string& userId, int itemType, int stockLevel, int stockThreshold)
{
    try
    {
        std::string query = "INSERT INTO inventory (user_id, item_type, stock_level, stock_threshold) "
                            "VALUES ($1, $2, $3, $4) "
                            "ON CONFLICT (user_id, item_type) DO UPDATE "
                            "SET stock_level = EXCLUDED.stock_level, stock_threshold = EXCLUDED.stock_threshold;";

        const char* paramValues[4] = {userId.c_str(), std::to_string(itemType).c_str(),
                                      std::to_string(stockLevel).c_str(), std::to_string(stockThreshold).c_str()};
        PGresult* res = PQexecParams(conn, query.c_str(), 4, nullptr, paramValues, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            Logger::getInstance().log("Database", "[ERROR] Error inserting/updating inventory: " +
                                                      std::string(PQerrorMessage(conn)));
            PQclear(res);
            return false;
        }

        PQclear(res);
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

std::string Database::findWarehouseForItem(int itemType, int quantityNeeded)
{
    try
    {
        std::string query =
            "SELECT i.user_id FROM inventory i "
            "WHERE i.user_id IN (SELECT u.user_id FROM users u WHERE u.user_id LIKE 'W%' AND u.is_online = TRUE) "
            "AND i.item_type = $1 AND i.stock_level >= $2 LIMIT 1;";

        const char* paramValues[2] = {std::to_string(itemType).c_str(), std::to_string(quantityNeeded).c_str()};
        PGresult* res = PQexecParams(conn, query.c_str(), 2, nullptr, paramValues, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            Logger::getInstance().log("Database",
                                      "[ERROR] Error finding warehouse for item: " + std::string(PQerrorMessage(conn)));
            PQclear(res);
            return "";
        }

        std::string warehouseId = "";
        if (PQntuples(res) > 0)
        {
            warehouseId = PQgetvalue(res, 0, 0);
        }

        PQclear(res);
        return warehouseId;
    }
    catch (const std::exception& e)
    {
        return "";
    }
}
