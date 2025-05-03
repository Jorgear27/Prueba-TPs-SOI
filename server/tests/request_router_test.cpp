#include "inventory.hpp"
#include "request_router.hpp"
#include <gtest/gtest.h>

// Test for routing client info
TEST(RequestRouterTest, RouteClientInfo)
{
    RequestRouter router;
    std::string request = R"({
        "type": "client_info",
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";

    int socket_test = 1; // Example socket value
    std::string response = router.routeRequest(request, socket_test);
    EXPECT_NE(response.find("\"status\":\"success\""), std::string::npos);
}

// Test for routing inventory updates
TEST(RequestRouterTest, RouteInventoryUpdate)
{
    RequestRouter router;
    std::string request = R"({
        "type": "inventory_update",
        "timestamp": "2025-04-01T12:10:00Z",
        "user_id": "W001",
        "inventory": [
            { "item_type": 0, "stock_level": 100 },
            { "item_type": 1, "stock_level": 50 }
        ]
    })";

    int socket_test = 1; // Example socket value
    std::string response = router.routeRequest(request, socket_test);
    EXPECT_NE(response.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(response.find("\"message\":\"Inventory updated\""), std::string::npos);
}

// Test for routing restock notices
TEST(RequestRouterTest, RouteRestockNotice)
{
    RequestRouter router;
    std::string request = R"({
        "type": "restock_notice",
        "user_id": "W001",
        "inventory": [
            { "item_type": 0, "stock_level": 200 },
            { "item_type": 1, "stock_level": 150 }
        ]
    })";

    int socket_test = 1; // Example socket value
    std::string response = router.routeRequest(request, socket_test);
    EXPECT_NE(response.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(response.find("\"message\":\"Restock notice processed\""), std::string::npos);
}

// Test for unknown request type
TEST(RequestRouterTest, UnknownRequestType)
{
    RequestRouter router;
    std::string request = R"({
        "type": "unknown_type"
    })";

    int socket_test = 1; // Example socket value
    std::string response = router.routeRequest(request, socket_test);
    EXPECT_NE(response.find("\"status\":\"unknown request\""), std::string::npos);
}

// Test for missing required fields
TEST(RequestRouterTest, MissingRequiredFields)
{
    RequestRouter router;
    std::string request = R"({
        "type": "client_info"
    })"; // Missing "hub_id" or "warehouse_id"

    int socket_test = 1; // Example socket value
    std::string response = router.routeRequest(request, socket_test);
    EXPECT_NE(response.find("\"status\":\"error\""), std::string::npos);
    EXPECT_NE(response.find("\"message\":\"Invalid JSON format or missing fields\""), std::string::npos);
}
