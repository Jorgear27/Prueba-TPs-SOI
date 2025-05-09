/**
 * @file database.hpp
 * @brief Header file for managing PostgreSQL database connections.
 *
 */

#ifndef database_H
#define database_H

#include "log.hpp"
#include <chrono>
#include <iostream>
#include <libpq-fe.h>
#include <thread>

/**
 * @brief Connection string for PostgreSQL database.
 *
 */
#define DB_INFO "host=localhost dbname=paranoid_db user=server password=server123 keepalives=1 keepalives_idle=10"

/**
 * @class Database
 * @brief A class to manage PostgreSQL database connections.
 * encapsulates the functionality required to connect to a PostgreSQL database using the `libpq` library.
 */
class Database
{
  public:
    /**
     * @brief Construct a new Database object
     *
     * @param connectionString
     */
    Database(const std::string& connectionString = DB_INFO);

    /**
     * @brief Destroys the Database object and disconnects from the database.
     *
     */
    ~Database();

    /**
     * @brief Checks if the database connection is active.
     * @return `true` if the connection is active, `false` otherwise.
     */
    bool is_connected();

    /**
     * @brief Disconnects from the PostgreSQL database.
     */
    void disconnect();

    /**
     * @brief Get the singleton instance of the Database class.
     *
     * This method provides access to a single instance of the Database class for use across all layers.
     *
     * @param connectionString The connection string for the PostgreSQL database.
     *
     * @return Database& Reference to the singleton Database instance.
     */
    static Database& getInstance(const std::string& connectionString = DB_INFO);

    /**
     * @brief Get the PostgreSQL connection object.
     *
     * This method provides access to the underlying `PGconn*` object used for
     * interacting with the PostgreSQL database.
     *
     * @param connectionString The connection string for the PostgreSQL database.
     *
     * @return PGconn* Pointer to the PostgreSQL connection object.
     */
    PGconn* getConnection(const std::string& connectionString = DB_INFO);

  private:
    /**
     * @brief A pointer to the PostgreSQL connection object.
     *
     * This member variable holds the connection object (`PGconn`) used by
     * the `libpq` library to manage the database connection.
     */
    PGconn* conn;

    /**
     * @brief Private static instance for the singleton pattern.
     *
     * This ensures that only one instance of the Database class exists
     */
    static Database* instance;
};

#endif
