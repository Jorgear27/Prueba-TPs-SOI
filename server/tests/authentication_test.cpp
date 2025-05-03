#include "authentication.hpp"
#include <gtest/gtest.h>

// Test for valid client information
TEST(AuthenticationTest, ValidClientInfo)
{

    Authentication auth;
    std::string validJson = R"({
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";
    int socket_test = 1; // Example socket value
    std::string response = auth.processClientInfo(validJson, socket_test);
    EXPECT_EQ(response, "{\"status\":\"success\",\"message\":\"Client information received and stored\"}");
}

// Test for invalid client information (missing hub_id/warehouse_id)
TEST(AuthenticationTest, MissingClientId)
{
    Authentication auth;
    std::string invalidJson = R"({
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";
    int socket_test = 1; // Example socket value
    std::string response = auth.processClientInfo(invalidJson, socket_test);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid JSON format or missing fields\"}");
}

// Test for invalid latitude
TEST(AuthenticationTest, InvalidLatitude)
{
    Authentication auth;
    std::string invalidJson = R"({
        "hub_id": "H001",
        "location": {
            "latitude": 100.0,
            "longitude": -74.0060
        }
    })";
    int socket_test = 1; // Example socket value
    std::string response = auth.processClientInfo(invalidJson, socket_test);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}

// Test for invalid longitude
TEST(AuthenticationTest, InvalidLongitude)
{
    Authentication auth;
    std::string invalidJson = R"({
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -200.0
        }
    })";
    int socket_test = 1; // Example socket value
    std::string response = auth.processClientInfo(invalidJson, socket_test);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}
