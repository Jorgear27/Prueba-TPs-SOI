#include "database.hpp"
#include <gtest/gtest.h>

// Test for successful connection
TEST(DatabaseTest, SuccessfulConnection)
{
    Database db;
    EXPECT_TRUE(db.is_connected());
}

// Test for disconnecting
TEST(DatabaseTest, Disconnect)
{
    Database db;
    db.disconnect();
    EXPECT_FALSE(db.is_connected());
}
