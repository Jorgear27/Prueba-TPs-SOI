#include "inventory.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Mock classes for dependencies
class MockLogger : public Logger
{
  public:
    MOCK_METHOD(void, log, (const std::string& component, const std::string& message), (override));
};

class MockDatabase : public Database
{
  public:
    MOCK_METHOD(bool, insertOrUpdateInventory, (const std::string& userId, int itemType, int stockLevel, int threshold),
                (override));
    MOCK_METHOD(std::string, findWarehouseForItem, (int itemType, int quantityNeeded), (override));
};

class InventoryManagerTest : public ::testing::Test
{
  protected:
    MockLogger mockLogger;
    MockDatabase mockDatabase;
    InventoryManager inventoryManager{mockLogger, mockDatabase};
};

// Test for handleInventoryUpdate
TEST_F(InventoryManagerTest, HandleInventoryUpdate_Success)
{
    std::string jsonData = R"({
                "user_id": "user123",
                "inventory": [
                        {"item_type": 1, "stock_level": 100, "threshold": 50},
                        {"item_type": 2, "stock_level": 200, "threshold": 100}
                ]
        })";

    EXPECT_CALL(mockDatabase, insertOrUpdateInventory("user123", 1, 100, 50)).WillOnce(::testing::Return(true));
    EXPECT_CALL(mockDatabase, insertOrUpdateInventory("user123", 2, 200, 100)).WillOnce(::testing::Return(true));
    EXPECT_CALL(mockLogger,
                log("InventoryManager", ::testing::StartsWith("[INFO] Inventory updated for user: user123")));

    inventoryManager.handleInventoryUpdate(jsonData);
}

// Test for handleInventoryUpdate with failure
TEST_F(InventoryManagerTest, HandleInventoryUpdate_Failure)
{
    std::string jsonData = R"({
                "user_id": "user123",
                "inventory": [
                        {"item_type": 1, "stock_level": 100, "threshold": 50}
                ]
        })";

    EXPECT_CALL(mockDatabase, insertOrUpdateInventory("user123", 1, 100, 50)).WillOnce(::testing::Return(false));
    EXPECT_CALL(mockLogger,
                log("InventoryManager", ::testing::StartsWith("[ERROR] Failed to update inventory for user: user123")));

    inventoryManager.handleInventoryUpdate(jsonData);
}

// Test for handleRestockNotice
TEST_F(InventoryManagerTest, HandleRestockNotice_Success)
{
    std::string jsonData = R"({
                "user_id": "user123",
                "item_type": 1,
                "stock_level": 150
        })";

    EXPECT_CALL(mockLogger,
                log("InventoryManager", ::testing::StartsWith("[INFO] Restock notice received for user: user123")));

    inventoryManager.handleRestockNotice(jsonData);
}

// Test for findWarehouseForItem
TEST_F(InventoryManagerTest, FindWarehouseForItem_Success)
{
    int itemType = 1;
    int quantityNeeded = 50;
    std::string warehouseId = "warehouse123";

    EXPECT_CALL(mockDatabase, findWarehouseForItem(itemType, quantityNeeded)).WillOnce(::testing::Return(warehouseId));

    std::string result = inventoryManager.findWarehouseForItem(itemType, quantityNeeded);
    EXPECT_EQ(result, warehouseId);
}

// Test for findWarehouseForItem with no warehouse found
TEST_F(InventoryManagerTest, FindWarehouseForItem_NoWarehouseFound)
{
    int itemType = 1;
    int quantityNeeded = 50;

    EXPECT_CALL(mockDatabase, findWarehouseForItem(itemType, quantityNeeded)).WillOnce(::testing::Return(""));
    EXPECT_CALL(mockLogger, log("InventoryManager", ::testing::StartsWith("[INFO] No warehouse found for item type:")));

    std::string result = inventoryManager.findWarehouseForItem(itemType, quantityNeeded);
    EXPECT_EQ(result, "");
}

// Test for getInstance method
TEST(InventoryManagerSingletonTest, GetInstance_ReturnsSameInstance)
{
    InventoryManager& instance1 = InventoryManager::getInstance();
    InventoryManager& instance2 = InventoryManager::getInstance();

    // Verify that both references point to the same instance
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(InventoryManagerTest, HandleInventoryUpdate_ExceptionThrown)
{
    std::string jsonData = R"({
                "user_id": "user123",
                "inventory": [
                        {"item_type": 1, "stock_level": 100, "threshold": 50}
                ]
        })";

    // Simulate an exception when calling insertOrUpdateInventory
    EXPECT_CALL(mockDatabase, insertOrUpdateInventory("user123", 1, 100, 50))
        .WillOnce(::testing::Throw(std::runtime_error("Database error")));

    EXPECT_CALL(mockLogger, log("InventoryManager",
                                ::testing::StartsWith("[ERROR] Exception occurred while handling inventory update:")));

    inventoryManager.handleInventoryUpdate(jsonData);
}

TEST_F(InventoryManagerTest, HandleRestockNotice_ExceptionThrown)
{
    std::string invalidJsonData = R"({
                "user_id": "user123",
                "item_type": "invalid_type", // Invalid type for item_type
                "stock_level": 150
        })";

    EXPECT_CALL(mockLogger, log("InventoryManager",
                                ::testing::StartsWith("[ERROR] Exception occurred while handling restock notice:")));

    inventoryManager.handleRestockNotice(invalidJsonData);
}

TEST_F(InventoryManagerTest, FindWarehouseForItem_ExceptionThrown)
{
    int itemType = 1;
    int quantityNeeded = 50;

    // Simulate an exception when calling findWarehouseForItem
    EXPECT_CALL(mockDatabase, findWarehouseForItem(itemType, quantityNeeded))
        .WillOnce(::testing::Throw(std::runtime_error("Database query error")));

    EXPECT_CALL(mockLogger,
                log("InventoryManager", ::testing::StartsWith("[ERROR] Exception occurred while finding warehouse:")));

    std::string result = inventoryManager.findWarehouseForItem(itemType, quantityNeeded);
    EXPECT_EQ(result, ""); // Expect an empty string as the return value
}
