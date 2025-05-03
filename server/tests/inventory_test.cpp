#include "inventory.hpp"
#include <gtest/gtest.h>

// Test for adding a new warehouse
TEST(InventoryManagerTest, AddNewWarehouse)
{
    InventoryManager inventoryManager;
    inventoryManager.handleNewWarehouse("W001", {{"item_type", 0}, {"stock_level", 100}});

    // Verify the warehouse was added
    std::string warehouseId = inventoryManager.findWarehouseForItem(0, 1); // No items yet
    EXPECT_EQ(warehouseId, "");                                            // No items in the warehouse
}

// Test for inventory update
TEST(InventoryManagerTest, InventoryUpdate)
{
    InventoryManager inventoryManager;
    std::string updateJson = R"({
        "user_id": "W001",
        "timestamp": "2025-04-01T12:10:00Z",
        "inventory": [
            { "item_type": 0, "stock_level": 100 },
            { "item_type": 1, "stock_level": 50 }
        ]
    })";

    inventoryManager.handleInventoryUpdate(updateJson);

    // Verify the inventory was updated
    std::string warehouseId = inventoryManager.findWarehouseForItem(0, 50);
    EXPECT_EQ(warehouseId, "W001");
}

// Test for finding a warehouse
TEST(InventoryManagerTest, FindWarehouse)
{
    InventoryManager inventoryManager;
    std::string updateJson = R"({
        "user_id": "W001",
        "inventory": [
            { "item_type": 0, "stock_level": 100 }
        ]
    })";

    inventoryManager.handleInventoryUpdate(updateJson);

    // Verify the warehouse can fulfill the request
    std::string warehouseId = inventoryManager.findWarehouseForItem(0, 50);
    EXPECT_EQ(warehouseId, "W001");
}

// Test for restock notice
TEST(InventoryManagerTest, RestockNotice)
{
    InventoryManager inventoryManager;
    std::string restockJson = R"({
        "user_id": "W001",
        "timestamp": "2025-04-01T12:10:00Z",
        "inventory": [
            { "item_type": 0, "stock_level": 200 },
            { "item_type": 1, "stock_level": 150 }
        ]
    })";

    inventoryManager.handleRestockNotice(restockJson);

    // Verify the inventory was updated
    std::string warehouseId = inventoryManager.findWarehouseForItem(0, 200);
    EXPECT_EQ(warehouseId, "W001");
}

// Test for invalid JSON input
TEST(InventoryManagerTest, InvalidJson)
{
    InventoryManager inventoryManager;
    std::string invalidJson = R"({
        "user_id": "W001",
        "inventory": [
            { "item_type": 0 }
        ]
    })"; // Missing "stock_level"

    EXPECT_NO_THROW(inventoryManager.handleInventoryUpdate(invalidJson));
}

// Test for multiple warehouses
TEST(InventoryManagerTest, MultipleWarehouses)
{
    InventoryManager inventoryManager;

    // Add inventory for W001
    std::string updateJson1 = R"({
        "user_id": "W001",
        "timestamp": "2025-04-01T12:10:00Z",
        "inventory": [
            { "item_type": 0, "stock_level": 100 }
        ]
    })";
    inventoryManager.handleInventoryUpdate(updateJson1);

    // Add inventory for W002
    std::string updateJson2 = R"({
        "user_id": "W002",
        "timestamp": "2025-04-01T12:10:00Z",
        "inventory": [
            { "item_type": 0, "stock_level": 200 }
        ]
    })";
    inventoryManager.handleInventoryUpdate(updateJson2);

    // Verify W002 is selected for quantities larger than W001
    std::string warehouseId = inventoryManager.findWarehouseForItem(0, 150);
    EXPECT_EQ(warehouseId, "W002");
}

// Test for no warehouse available
TEST(InventoryManagerTest, NoWarehouseAvailable)
{
    InventoryManager inventoryManager;

    // Add inventory for W001
    std::string updateJson = R"({
        "user_id": "W001",
        "inventory": [
            { "item_type": 0, "stock_level": 50 }
        ]
    })";
    inventoryManager.handleInventoryUpdate(updateJson);

    // Verify no warehouse can fulfill the request
    std::string warehouseId = inventoryManager.findWarehouseForItem(0, 100);
    EXPECT_EQ(warehouseId, "");
}
