#include "database.hpp"

// Constructor
Database::Database()
{
    // Connect to the database using the connection string
    conn = PQconnectdb(DB_INFO);

    // Check if the connection was successful
    if (PQstatus(conn) != CONNECTION_OK)
    {
        // Log the error message
        Logger::getInstance().log("Database",
                                  "[ERROR] Connection to database failed: " + std::string(PQerrorMessage(conn)));
        std::cerr << "[ERROR] Connection to database failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn); // If fails, free the failed connection
        conn = nullptr; // Ensure the pointer does not point to an invalid connection
    }
    else
    {
        // If the connection is successful log the success message
        Logger::getInstance().log("Database", "[INFO] Connected to paranoid_db database successfully.");
        std::cout << "[INFO] Connected to paranoid_db database successfully. \n";
    }
}

// Destructor
Database::~Database()
{
    disconnect(); // Call the method to close the connection
}

// Check if the connection is active
bool Database::is_connected()
{
    // Return true if the pointer is not null and the connection is in OK status
    return conn != nullptr && PQstatus(conn) == CONNECTION_OK;
}

// Close the connection to the database
void Database::disconnect()
{
    // Check if there is an active connection
    if (conn)
    {
        PQfinish(conn); // Close the connection and release resources
        conn = nullptr; // Ensure the pointer does not point to a closed connection
    }
}

// Singleton: return the instance of the Database class
Database& Database::getInstance()
{
    static Database instance;
    return instance;
}

PGconn* Database::getConnection()
{
    if (is_connected())
    {
        return conn; // Return the existing connection if it's active
    }

    // Attempt to reconnect if the connection is lost
    // Log the error message
    Logger::getInstance().log("Database", "[ERROR] Database connection lost. Attempting to reconnect...");
    std::cerr << "[ERROR] Database connection lost. Attempting to reconnect...\n";
    disconnect(); // Close the existing connection if any

    // Reconnect to the database
    conn = PQconnectdb(DB_INFO);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        // Log the error message
        Logger::getInstance().log("Database",
                                  "[ERROR] Reconnection to database failed: " + std::string(PQerrorMessage(conn)));
        std::cerr << "[ERROR] Failed to reconnect: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        conn = nullptr;
    }

    // Return the connection object
    return conn;
}
