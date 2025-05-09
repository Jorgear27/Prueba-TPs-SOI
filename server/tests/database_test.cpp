#include "database.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

// Test fixture for Database
class DatabaseTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Ensure the database connection is initialized before each test
        db = &Database::getInstance();
        db->getConnection();
    }

    void TearDown() override
    {
        // Disconnect the database connection after each test
        db->disconnect();
        db = nullptr; // Reset the pointer to avoid dangling reference
    }

    Database* db;
};

// Test the singleton instance
TEST_F(DatabaseTest, SingletonInstance)
{
    Database* instance1 = &Database::getInstance();
    Database* instance2 = &Database::getInstance();
    ASSERT_EQ(instance1, instance2) << "Singleton instances should be the same.";
}

// Test the connection to the database
TEST_F(DatabaseTest, ConnectionSuccess)
{
    ASSERT_TRUE(db->is_connected()) << "Database should be connected successfully.";
}

// Test disconnecting from the database
TEST_F(DatabaseTest, Disconnect)
{
    db->disconnect();
    ASSERT_FALSE(db->is_connected()) << "Database should be disconnected.";
}

// Test reconnecting to the database
TEST_F(DatabaseTest, Reconnect)
{
    db->disconnect();
    ASSERT_FALSE(db->is_connected()) << "Database should be disconnected.";
    db->getConnection();
    ASSERT_TRUE(db->is_connected()) << "Database should reconnect successfully.";
}

// Test getConnection when already connected
TEST_F(DatabaseTest, GetConnectionWhenConnected)
{
    PGconn* conn = db->getConnection();
    ASSERT_NE(conn, nullptr) << "Connection object should not be null when connected.";
    ASSERT_TRUE(db->is_connected()) << "Database should remain connected.";
}

// Test database constructor with invalid connection string
TEST_F(DatabaseTest, ConstructorConnectionFailure)
{
    // Use an invalid connection string
    std::string invalidConnectionString =
        "host=invalid_host dbname=invalid_db user=invalid_user password=invalid_password";

    // Create a new instance of the database with the invalid connection string
    Database dbInstance(invalidConnectionString);

    // Verify that the connection is not established
    ASSERT_FALSE(dbInstance.is_connected()) << "Database connection should fail with invalid connection string.";

    // Clean up
    dbInstance.disconnect(); // Ensure the connection is closed
}

// Test getConnection with reconnection failure
TEST_F(DatabaseTest, ReconnectionFailure)
{
    // Use an invalid connection string
    std::string invalidConnectionString =
        "host=invalid_host dbname=invalid_db user=invalid_user password=invalid_password";

    // Disconnect the database to force reconnection
    db->disconnect();

    // Attempt to get a connection, which should fail
    PGconn* conn = db->getConnection(invalidConnectionString);
    ASSERT_EQ(conn, nullptr) << "Connection object should be null after failed reconnection.";
    ASSERT_FALSE(db->is_connected()) << "Database should remain disconnected after failed reconnection.";
}
