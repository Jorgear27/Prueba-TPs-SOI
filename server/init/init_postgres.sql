GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO server;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO server;
GRANT ALL PRIVILEGES ON ALL FUNCTIONS IN SCHEMA public TO server;

CREATE DATABASE paranoid_db;

-- Connect to the database
\c paranoid_db;

--USERS TABLE (for the server to recognize his clients)
CREATE TABLE IF NOT EXISTS users (
    user_id TEXT PRIMARY KEY,  -- Format: H001, W002, etc
    latitude DOUBLE PRECISION NOT NULL,
    longitude DOUBLE PRECISION NOT NULL,
    is_online BOOLEAN NOT NULL DEFAULT FALSE -- Define if the user is online or offline
);

-- INVENTORY TABLE (to know the stock in the warehouses)
CREATE TABLE IF NOT EXISTS inventory (
    user_id TEXT REFERENCES users(user_id),
    item_type INT NOT NULL,  -- 0:Medicines, 1:Food, 3:Clothing
    stock_level INT NOT NULL, -- Current stock level
    stock_threshold INT NOT NULL, -- Minimum stock level before reordering
    PRIMARY KEY(user_id, item_type), -- Unique per item per warehouse
    CHECK (user_id LIKE 'W%') -- Validate that the user_id is a warehouse
);

-- ORDERS TABLE (for orders that were shipped from the hub)
CREATE TABLE IF NOT EXISTS orders (
    order_id TEXT NOT NULL, -- Order ID, can be repeated for multiple items in the same order
    user_id TEXT REFERENCES users(user_id),  -- Hub issuing the order
    item_type INT NOT NULL, -- 0:Medicines, 1:Food, 3:Clothing
    quantity INT NOT NULL, -- Quantity of items ordered
    status TEXT NOT NULL CHECK (status IN ('Logged', 'Pending', 'Approved', 'Requested', 'Shipped', 'Delivered', 'Canceled')),
    CHECK (user_id LIKE 'H%'), -- Validate that the user_id is a hub
    PRIMARY KEY (order_id, item_type) -- Composite primary key to allow multiple items per order
);

-- Status:
-- Logged: Order has been logged in the system
-- Pending: Order is pending approval (in window of cancellation)
-- Approved: Order has been approved by the hub
-- Requested: Order has been requested from the warehouse but not yet shipped
-- Shipped: Order has been shipped from the warehouse
-- Delivered: Order has been delivered to the client by the hub
-- Canceled: Order has been canceled by the hub (before approval) or by the server (after approval)
