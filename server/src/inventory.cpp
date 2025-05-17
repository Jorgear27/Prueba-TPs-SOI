#include "inventory.hpp"

void InventoryManager::handleInventoryUpdate(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Extract fields
        std::string userId = message.at("user_id");
        auto inventoryList = message.at("inventory");

        // Add a new entry for the warehouse
        for (const auto& item : inventoryList)
        {
            int itemType = item.at("item_type");
            int stockLevel = item.at("stock_level");
            int stockThreshold = item.at("threshold");

            // Use Database to insert or update inventory
            if (!database.insertOrUpdateInventory(userId, itemType, stockLevel, stockThreshold))
            {
                logger.log("InventoryManager", "[ERROR] Failed to update inventory for user: " + userId);
                std::cerr << "[ERROR] Failed to update inventory for user: " << userId << "\n";
                return;
            }
        }
        // Log the updated inventory
        logger.log("InventoryManager", "[INFO] Inventory updated for user: " + userId);
        std::cout << "[INFO] Inventory updated for user: " << userId << "\n";
    }
    catch (const std::exception& e)
    {
        logger.log("InventoryManager",
                   "[ERROR] Exception occurred while handling inventory update: " + std::string(e.what()));
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
        std::string userId = message.at("user_id");
        int itemType = message.at("item_type");
        int stockLevel = message.at("stock_level");

        // Log the restock notice
        logger.log("InventoryManager", "[INFO] Restock notice received for user: " + userId + ", item type: " +
                                           std::to_string(itemType) + ", stock level: " + std::to_string(stockLevel));
        return;
    }
    catch (const std::exception& e)
    {
        logger.log("InventoryManager",
                   "[ERROR] Exception occurred while handling restock notice: " + std::string(e.what()));
        return;
    }
}

std::string InventoryManager::findWarehouseForItem(int itemType, int quantityNeeded)
{
    try
    {
        // Use Database to find a warehouse
        std::string warehouseId = database.findWarehouseForItem(itemType, quantityNeeded);

        if (warehouseId.empty())
        {
            logger.log("InventoryManager", "[INFO] No warehouse found for item type: " + std::to_string(itemType) +
                                               ", quantity needed: " + std::to_string(quantityNeeded));
            std::cerr << "[INFO] No warehouse found for item type: " << itemType
                      << ", quantity needed: " << quantityNeeded << "\n";
        }

        return warehouseId;
    }
    catch (const std::exception& e)
    {
        logger.log("InventoryManager", "[ERROR] Exception occurred while finding warehouse: " + std::string(e.what()));
        return "";
    }
}

// Singleton instance
InventoryManager& InventoryManager::getInstance()
{
    static InventoryManager instance; // Guaranteed to be destroyed
    return instance;                  // Instantiated on first use
}
