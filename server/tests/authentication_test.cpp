#include "authentication.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>

#define SOCKET_TEST 1

class MockDatabase : public Database
{
  public:
    MOCK_METHOD(bool, insertOrUpdateUser, (const std::string& userId, double latitude, double longitude), (override));
    MOCK_METHOD(bool, updateUserOnlineStatus, (const std::string& userId, bool isOnline), (override));
};

class AuthenticationManagerTest : public ::testing::Test
{
  protected:
    MockDatabase mockDb;         // Mock
    Authentication auth{mockDb}; // Authentication instance with mock database
};

// Test for valid client information
TEST_F(AuthenticationManagerTest, ValidClientInfo)
{
    std::string validJson = R"({
        "type": "client_info", 
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060))
        .Times(1)
        .WillOnce(testing::Return(true)); // Simulate successful database operation
    std::string response = auth.processClientInfo(validJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"success\",\"message\":\"Client information received and stored\"}");
}

// Test for invalid client information (missing timestamp)
TEST_F(AuthenticationManagerTest, MissingTimestamp)
{
    std::string invalidJson = R"({
        "type": "client_info", 
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060)).Times(0); // Must return failure before DB call
    std::string response = auth.processClientInfo(invalidJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}

// Test for invalid client information (missing hub_id/warehouse_id)
TEST_F(AuthenticationManagerTest, MissingClientId)
{
    std::string invalidJson = R"({
        "type": "client_info", 
        "timestamp": "2025-04-01T12:01:00Z",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060)).Times(0); // Must return failure before DB call
    std::string response = auth.processClientInfo(invalidJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}

// Test for invalid location (missing array)
TEST_F(AuthenticationManagerTest, MissingLocation)
{
    std::string invalidJson = R"({
        "type": "client_info", 
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001"
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060)).Times(0); // Must return failure before DB call
    std::string response = auth.processClientInfo(invalidJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}

// Test for invalid JSON format
TEST_F(AuthenticationManagerTest, InvalidJsonFormat)
{
    std::string invalidJson = R"({
        "type": "client_info",
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": "invalid_latitude",
            "longitude": -74.0060
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060)).Times(0); // Must return failure before DB call
    std::string response = auth.processClientInfo(invalidJson, SOCKET_TEST);
    EXPECT_THAT(response, ::testing::HasSubstr("Invalid JSON format or missing fields"));
}

// Test for database retries and failure
TEST_F(AuthenticationManagerTest, DatabaseRetriesAndFails)
{
    std::string validJson = R"({
        "type": "client_info", 
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";

    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060))
        .Times(3)
        .WillRepeatedly(testing::Return(false)); // Simulate failure for all retries

    std::string response = auth.processClientInfo(validJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Failed to process new client after retries\"}");
}

// Test for database success after failure
TEST_F(AuthenticationManagerTest, DatabaseFailsThenSucceeds)
{
    std::string validJson = R"({
        "type": "client_info", 
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": 40.7128,
            "longitude": -74.0060
        }
    })";

    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060))
        .WillOnce(testing::Return(false))
        .WillOnce(testing::Return(false))
        .WillOnce(testing::Return(true));

    std::string response = auth.processClientInfo(validJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"success\",\"message\":\"Client information received and stored\"}");
}
