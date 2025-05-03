#include "inventory.hpp"
#include "orders.hpp"
#include <gtest/gtest.h>

// Test for empty order requests
TEST(OrderManagerTest, EmptyOrderRequest)
{
    OrderManager orderManager;

    // Create an empty order
    std::vector<std::pair<int, int>> itemsNeeded = {};
    std::string orderID = "ORD001";
    std::string response = orderManager.supplyRequest(orderID, itemsNeeded);

    // Verify the response is valid but contains no items
    EXPECT_NE(response.find("\"items_needed\":[]"), std::string::npos);
}

// Test for invalid input
TEST(OrderManagerTest, InvalidInput)
{
    OrderManager orderManager;

    // Create an order with invalid input (negative quantity)
    std::vector<std::pair<int, int>> itemsNeeded = {{0, -50}, {1, -30}};
    std::string orderID = "ORD001";
    std::string response = orderManager.supplyRequest(orderID, itemsNeeded);

    // Verify the response contains "fulfilled_by":"none" for invalid items
    EXPECT_NE(response.find("\"fulfilled_by\":\"none\""), std::string::npos);
}

// Test for edge cases (zero quantity)
TEST(OrderManagerTest, ZeroQuantityOrder)
{
    OrderManager orderManager;

    // Create an order with zero quantity
    std::vector<std::pair<int, int>> itemsNeeded = {{0, 0}, {1, 0}};
    std::string orderID = "ORD001";
    std::string response = orderManager.supplyRequest(orderID, itemsNeeded);

    // Verify the response contains "fulfilled_by":"none" for zero quantity
    EXPECT_NE(response.find("\"fulfilled_by\":\"none\""), std::string::npos);
}
