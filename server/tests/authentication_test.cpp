#include "authentication.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define SOCKET_TEST 1

class MockDatabase : public Database
{
  public:
    MOCK_METHOD(bool, insertOrUpdateUser, (const std::string& userId, double latitude, double longitude), (override));
    MOCK_METHOD(bool, updateUserOnlineStatus, (const std::string& userId, bool isOnline), (override));
};

class MockSender : public Sender
{
  public:
    MOCK_METHOD(void, removeConnection, (const std::string& userId), (override));
};

class AuthenticationManagerTest : public ::testing::Test
{
  protected:
    MockDatabase mockDb;                     // Mock Database
    MockSender mockSender;                   // Mock Sender
    Authentication auth{mockDb, mockSender}; // Authentication instance with mock database and sender
};

// Test for validation of client information missing latitude and longitude
TEST_F(AuthenticationManagerTest, MissingLatitudeLongitude)
{
    std::string invalidJson = R"({
        "type": "client_info",
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": null,
            "longitude": null
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 40.7128, -74.0060)).Times(0); // Must return failure before DB call
    std::string response = auth.processClientInfo(invalidJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}

// Test for validation of client information latitude and longitude out of range
TEST_F(AuthenticationManagerTest, InvalidLatitudeLongitude)
{
    std::string invalidJson = R"({
        "type": "client_info",
        "timestamp": "2025-04-01T12:01:00Z",
        "hub_id": "H001",
        "location": {
            "latitude": 100.0,
            "longitude": -200.0
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("H001", 100.0, -200.0)).Times(0); // Must return failure before DB call
    std::string response = auth.processClientInfo(invalidJson, SOCKET_TEST);
    EXPECT_EQ(response, "{\"status\":\"error\",\"message\":\"Invalid client information\"}");
}

// Test for valid client information
TEST_F(AuthenticationManagerTest, ValidHubInfo)
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

// Test for valid client information
TEST_F(AuthenticationManagerTest, ValidWhInfo)
{
    std::string validJson = R"({
        "type": "client_info",
        "timestamp": "2025-04-01T12:01:00Z",
        "warehouse_id": "W001",
        "location": {
            "latitude": 20.3050,
            "longitude": -14.0527
        }
    })";
    EXPECT_CALL(mockDb, insertOrUpdateUser("W001", 20.3050, -14.0527))
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

TEST_F(AuthenticationManagerTest, HandleClientDisconnectionSuccess)
{
    std::string jsonData = R"({
        "user_id": "H001",
        "timestamp": "2025-04-01T12:01:00Z"
    })";

    // Expect the database to update the user's online status successfully
    EXPECT_CALL(mockDb, updateUserOnlineStatus("H001", false)).Times(1).WillOnce(testing::Return(true));

    // Expect the Sender to remove the connection
    EXPECT_CALL(mockSender, removeConnection("H001")).Times(1);

    // Call the method
    auth.handleClientDisconnection(jsonData, SOCKET_TEST);
}

TEST_F(AuthenticationManagerTest, HandleClientDisconnectionDatabaseFailure)
{
    std::string jsonData = R"({
        "user_id": "H001",
        "timestamp": "2025-04-01T12:01:00Z"
    })";

    // Simulate a failure in updating the user's online status
    EXPECT_CALL(mockDb, updateUserOnlineStatus("H001", false)).Times(1).WillOnce(testing::Return(false));

    // Expect the Sender to still remove the connection
    EXPECT_CALL(mockSender, removeConnection("H001")).Times(1);

    // Call the method
    auth.handleClientDisconnection(jsonData, SOCKET_TEST);
}
