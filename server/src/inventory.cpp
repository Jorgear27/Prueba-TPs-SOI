#include "inventory.hpp"

void InventoryManager::handleInventoryUpdate(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Extract fields
        std::string userId = message.at("warehouse_id");
        auto inventoryList = message.at("inventory");

        PGconn* conn = Database::getInstance().getConnection();

        // Add a new entry for the warehouse
        for (const auto& item : inventoryList)
        {
            int itemType = item.at("item_type");
            int stockLevel = item.at("stock_level");
            int stockThreshold = item.at("threshold");

            // Build the SQL query to insert the new warehouse and its inventory
            std::string query = "INSERT INTO inventory (user_id, item_type, stock_level, stock_threshold) VALUES ('" +
                                userId + "', " + std::to_string(itemType) + ", " + std::to_string(stockLevel) + ", " +
                                std::to_string(stockThreshold) +
                                ") ON CONFLICT (user_id, item_type) DO UPDATE SET "
                                "stock_level = EXCLUDED.stock_level, stock_threshold = EXCLUDED.stock_threshold;";

            // Execute the query and check the result
            PGresult* res = PQexec(conn, query.c_str());
            if (PQresultStatus(res) != PGRES_COMMAND_OK)
            {
                // Log the error updating the inventory
                Logger::getInstance().log("InventoryManager",
                                          "[ERROR] Failed to update inventory: " + std::string(PQerrorMessage(conn)));
                std::cerr << "[ERROR] Failed to update inventory: " << std::string(PQerrorMessage(conn)) << "\n";
                PQclear(res);
                return;
            }

            // Clean up the result for a new query
            PQclear(res);
        }
        // Log the updated inventory
        Logger::getInstance().log("InventoryManager", "[INFO] Inventory updated for user: " + userId);
        std::cout << "[INFO] Inventory updated for user: " << userId << "\n";
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("InventoryManager", "[ERROR] Exception occurred while handling inventory update: " +
                                                          std::string(e.what()));
        std::cerr << "[ERROR] Failed to handle inventory update: " << e.what() << "\n";
        return;
    }
}

void InventoryManager::handleRestockNotice(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Extract fields
        std::string userId = message.at("warehouse_id");
        int itemType = message.at("item_type");
        int stockLevel = message.at("stock_level");

        // Log the restock notice
        Logger::getInstance().log("InventoryManager", "[INFO] Restock notice received for user: " + userId +
                                                          ", item type: " + std::to_string(itemType) +
                                                          ", stock level: " + std::to_string(stockLevel));
        return;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("InventoryManager",
                                  "[ERROR] Exception occurred while handling restock notice: " + std::string(e.what()));
        std::cerr << "[ERROR] Failed to handle restock notice: " << e.what() << "\n";
        return;
    }
}

std::string InventoryManager::findWarehouseForItem(int itemType, int quantityNeeded)
{
    // Query the database to find a warehouse that can fulfill the request
    PGconn* conn = Database::getInstance().getConnection();

    // Build the SQL query to find a warehouse with sufficient stock
    std::string query = "SELECT i.user_id FROM inventory i WHERE i.user_id IN(SELECT u.user_id FROM users u WHERE "
                        "u.user_id LIKE 'W%' AND u.is_online = TRUE) AND i.item_type = " +
                        std::to_string(itemType) + " AND i.stock_level >= " + std::to_string(quantityNeeded) +
                        " LIMIT 1;";

    // Execute the query and check the result
    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        Logger::getInstance().log("InventoryManager",
                                  "Error finding a warehouse , executing query: " + std::string(PQerrorMessage(conn)));
        std::cerr << "[ERROR] Failed to find a warehouse: " << std::string(PQerrorMessage(conn)) << "\n";
        PQclear(res);
        return "";
    }

    // Recover the warehouse ID from the result
    std::string warehouseId = "";
    if (PQntuples(res) > 0)
    {
        warehouseId = PQgetvalue(res, 0, 0); // Get the user_id
    }

    PQclear(res);
    return warehouseId;
}
