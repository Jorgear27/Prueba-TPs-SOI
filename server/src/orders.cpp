#include "orders.hpp"

void OrderManager::handleNewOrder(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Validate the order details
        if (!validateOrderDetails(message))
        {
            std::cerr << "[ERROR] Invalid order details.\n";
            return; // Exit early if validation fails
        }

        // Extract fields
        std::string hubId = message.at("hub_id");
        std::string orderId = message.at("order_id");
        auto orderItemsList = message.at("items_needed");

        // Add the new order to the database
        for (const auto& item : orderItemsList)
        {
            int itemType = item.at("item_type");
            int quantityRequest = item.at("quantity");

            if (!Database::getInstance().insertOrUpdateOrder(orderId, hubId, itemType, quantityRequest))
            {
                Logger::getInstance().log("OrderManager", "Error inserting/updating order.");
                std::cerr << "[ERROR] Failed to insert/update order.\n";
                return;
            }
        }

        // Launch a separate thread to handle the status update
        std::thread([orderId]() {
            try
            {
                // Sleep for 30 seconds (cancellation window)
                std::this_thread::sleep_for(std::chrono::seconds(30));
                std::string status = Database::getInstance().getOrderStatus(orderId);

                if (status == "Pending")
                {
                    Database::getInstance().updateOrderStatus(orderId, "Approved");
                    std::cout << "[INFO] Order " << orderId << " status updated to 'Approved'.\n";
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "[ERROR] Exception in thread: " << e.what() << "\n";
            }
        }).detach(); // Detach the thread to allow it to run independently

        Logger::getInstance().log("OrderManager", "New order added for user: " + hubId);
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("OrderManager", "Error creating order: " + std::string(e.what()));
        std::cerr << "[Order] Failed to process supply request: " << e.what() << "\n";
        return;
    }
}

void OrderManager::processApprovedOrders()
{
    while (true)
    {
        try
        {
            // Query the database for orders with status "Approved"
            std::vector<std::string> approvedOrders = Database::getInstance().getApprovedOrders();

            for (const auto& orderId : approvedOrders)
            {
                // Fetch order details
                nlohmann::json orderDetails = getOrderDetails(orderId);

                // Extract items needed
                std::vector<std::pair<int, int>> itemsNeeded;
                for (const auto& item : orderDetails["items_needed"])
                {
                    int itemType = item["item_type"];
                    int quantity = item["quantity"];
                    itemsNeeded.emplace_back(itemType, quantity);
                }

                // Call supplyRequest to fulfill the order
                std::string supplyResponse = supplyRequest(orderId, itemsNeeded);

                // Update the order status to "Requested"
                updateOrderStatus(orderId, "Requested");

                Logger::getInstance().log("OrderManager", "Processed approved order: " + orderId);
            }
        }
        catch (const std::exception& e)
        {
            Logger::getInstance().log("OrderManager", "Error processing approved orders: " + std::string(e.what()));
        }

        // Wait for 10 seconds before checking for new approved orders
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

std::string OrderManager::supplyRequest(const std::string orderId, const std::vector<std::pair<int, int>>& itemsNeeded)
{
    try
    {
        // Create the supply request JSON
        nlohmann::json supplyRequest;
        supplyRequest["type"] = "supply_request";
        supplyRequest["timestamp"] = "2025-04-01T12:00:00Z"; // Example timestamp
        supplyRequest["order_id"] = orderId;

        // Construct the items_needed array
        nlohmann::json itemsArray = nlohmann::json::array();
        for (const auto& item : itemsNeeded)
        {
            int itemType = item.first;
            int quantityNeeded = item.second;

            // Add each item as a JSON object
            itemsArray.push_back({{"item_type", itemType}, {"quantity", quantityNeeded}});
        }
        supplyRequest["items_needed"] = itemsArray;

        for (const auto& item : itemsNeeded)
        {
            int itemType = item.first;
            int quantityNeeded = item.second;

            // Ask InventoryManager for a warehouse that can fulfill the request
            InventoryManager inventoryManager; // Use InventoryManager to query inventory
            std::string warehouseId = inventoryManager.findWarehouseForItem(itemType, quantityNeeded);
            if (!warehouseId.empty())
            {
                if (Sender::getInstance().sendMessageToClient(warehouseId, supplyRequest.dump()) != -1)
                {
                    printf("[INFO] Supply request sent to warehouse: %s\n", warehouseId.c_str());
                    // Log the operation
                    Logger::getInstance().log("OrderManager",
                                              "[ORDER] Fulfilled request for item_type: " + std::to_string(itemType) +
                                                  ", quantity: " + std::to_string(quantityNeeded) +
                                                  " from warehouse: " + warehouseId);
                    std::cout << "[Order] Fulfilled request for item_type: " << itemType
                              << ", quantity: " << quantityNeeded << " from warehouse: " << warehouseId << "\n";
                    // Add the fulfilled item to the supply request JSON
                    supplyRequest["items_needed"].push_back(
                        {{"item_type", itemType}, {"quantity", quantityNeeded}, {"fulfilled_by", warehouseId}});
                    //}
                }
                else
                {
                    Logger::getInstance().log("OrderManager",
                                              "[ERROR] Failed to send supply request to warehouse: " + warehouseId);
                }
            }
            else
            {
                Logger::getInstance().log("OrderManager",
                                          "[ERROR] No warehouse found for item_type: " + std::to_string(itemType) +
                                              ", quantity: " + std::to_string(quantityNeeded));
                // Add the unfulfilled item to the supply request JSON
                supplyRequest["items_needed"].push_back(
                    {{"item_type", itemType}, {"quantity", quantityNeeded}, {"fulfilled_by", "none"}});
            }
        }
        return supplyRequest.dump();
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("OrderManager", " [ERROR] Exception in supplyRequest: " + std::string(e.what()));
        std::cerr << "[ERROR] Failed to create supply request: " << e.what() << "\n";
        return "{\"status\":\"error\",\"message\":\"Failed to process supply request\"}";
    }
}

void OrderManager::handleOrderDispatch(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Extract fields
        std::string orderId = message.at("order_id");
        std::string status = message.at("status"); // shipped or canceled
        auto itemsShipped = message.at("items_shipped");

        // JSON object to send to the hub for notification
        nlohmann::json orderDistribution;
        orderDistribution["type"] = "order_for_distribution";
        orderDistribution["timestamp"] = "2025-04-01T12:10:00Z";
        orderDistribution["order_id"] = orderId;
        orderDistribution["status"] = status;
        orderDistribution["items_shipped"] = itemsShipped;

        nlohmann::json orderDetails = getOrderDetails(orderId);
        std::string hubId = orderDetails.at("user_id");

        // Notify the hub about the order dispatch Sender sender;
        if (Sender::getInstance().sendMessageToClient(hubId, orderDistribution.dump()) != -1)
        {
            // Log the operation
            Logger::getInstance().log("OrderManager",
                                      "[ORDER] Order dispatched: " + orderId + ", new status: " + status);
            std::cout << "[ORDER] Order dispatched: " << orderId << ", new status: " << status << "\n";
        }
        else
        {
            // Log the error
            Logger::getInstance().log("OrderManager",
                                      "[ERROR] Failed to send order dispatch notification to hub: " + hubId);
            std::cerr << "[ERROR] Failed to send order dispatch notification to hub.\n";
            return; // Exit early if sending fails
        }

        // Update the order status in the database
        updateOrderStatus(orderId, status);

        return;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("OrderManager", "Error processing order dispatch: " + std::string(e.what()));
        std::cerr << "[ORDER] Failed to process order dispatch: " << e.what() << "\n";
        return;
    }
}

std::string OrderManager::handleCancelation(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Extract fields
        std::string orderId = message.at("order_id");

        std::string details = getOrderDetails(orderId).dump();

        json response_details = json::parse(details);
        std::string status = response_details.at("status");

        // Check if the order_id exists
        if (status == "error")
        {
            std::cerr << "[ERROR] Order ID not found.\n";
            return "{\"status\":\"error\",\"message\":\"Invalid client ID\"}"; // Exit early if order ID is not found
        }

        if (status == "Pending")
        {
            std::string status = "Canceled";
            updateOrderStatus(orderId, status);
        }
        else
        {
            std::cout << "[ERROR] The order was already approved, it cannot be canceled. \n";
            Logger::getInstance().log("OrderManager", "The order was already approved, it cannot be canceled.");
            return "{\"status\":\"error\",\"message\":\"The order was already approved, it cannot be canceled\"}";
        }

        // Log the operation
        Logger::getInstance().log("OrderManager", "Order canceled: " + orderId);
        std::cout << "[ORDER] Order canceled: " << orderId << "\n";
        return "{\"status\":\"success\",\"message\":\"The order was canceled\"}";
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("OrderManager", "Error processing order cancelation: " + std::string(e.what()));
        std::cerr << "[ORDER] Failed to process order cancelation: " << e.what() << "\n";
        return "{\"status\":\"error\",\"message\":\"Failed to process order cancelation\"}";
    }
}

std::string OrderManager::handleOrderStatusQuery(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Validate the JSON structure
        if (!message.contains("type") || !message.at("type").is_string())
        {
            throw std::runtime_error("[ERROR] 'type' field must be a string.");
        }
        if (!message.contains("hub_id") || !message.at("hub_id").is_string())
        {
            throw std::runtime_error("[ERROR] 'hub_id' field must be a string.");
        }
        if (!message.contains("timestamp") || !message.at("timestamp").is_string())
        {
            throw std::runtime_error("[ERROR] 'timestamp' field must be a string.");
        }
        if (!message.contains("order_id") || !message.at("order_id").is_string())
        {
            throw std::runtime_error("[ERROR] 'order_id' field must be a string.");
        }

        // Extract fields
        std::string orderId = message.at("order_id");

        // Fetch order details
        json orderDetails = getOrderDetails(orderId);

        // Return the order details
        return orderDetails.dump();
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("OrderManager", "Error processing order status query: " + std::string(e.what()));
        std::cerr << "[ERROR] Failed to process order status query: " << e.what() << "\n";
        return json{{"status", "error"}, {"message", e.what()}}.dump();
    }
}

void OrderManager::deliveryUpdate(const std::string& jsonData)
{
    try
    {
        // Parse the JSON data
        json message = json::parse(jsonData);

        // Validate the order details
        if (!validateOrderUpdate(message))
        {
            std::cerr << "[ERROR] Invalid order details.\n";
            return; // Exit early if validation fails
        }

        // Extract fields
        std::string orderId = message.at("order_id");
        std::string status = message.at("status");

        // Update the order status in the database
        updateOrderStatus(orderId, status);

        // Log the operation
        Logger::getInstance().log("OrderManager", "Order dispatched: " + orderId + ", new status: " + status);
        return;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("OrderManager", "Error processing delivery update: " + std::string(e.what()));
        std::cerr << "[ORDER] Failed to process order dispatch: " << e.what() << "\n";
        return;
    }
}

nlohmann::json OrderManager::getOrderDetails(const std::string& orderId)
{
    return Database::getInstance().getOrderDetails(orderId);
}

void OrderManager::updateOrderStatus(const std::string& orderId, const std::string& newStatus)
{
    if (!Database::getInstance().updateOrderStatus(orderId, newStatus))
    {
        Logger::getInstance().log("OrderManager", "Failed to update order status for order: " + orderId);
        std::cerr << "[ERROR] Failed to update order status for order: " << orderId << "\n";
    }
}

bool OrderManager::validateOrderDetails(const json& message)
{
    // Check if "hub_id" is present and not null
    if (!message.contains("hub_id") || message.at("hub_id").is_null())
    {
        std::cerr << "[ERROR] Missing or null hub_id.\n";
        return false;
    }

    // Check if "order_id" is present and not null
    if (!message.contains("order_id") || message.at("order_id").is_null())
    {
        std::cerr << "[ERROR] Missing or null order_id.\n";
        return false;
    }

    // Check if "items_needed" is present and is an array
    if (!message.contains("items_needed") || message.at("items_needed").is_null() ||
        !message.at("items_needed").is_array())
    {
        std::cerr << "[ERROR] Missing or invalid items_needed array.\n";
        return false;
    }

    // Validate items array (each item should have "item_type" and "quantity")
    for (const auto& item : message.at("items_needed"))
    {
        if (!item.contains("item_type") || !item.contains("quantity") || item.at("item_type").is_null() ||
            item.at("quantity").is_null())
        {
            std::cerr << "[ERROR] Missing or null item_type or quantity in item.\n";
            return false;
        }
    }

    return true; // All checks passed, data is valid
}

bool OrderManager::validateOrderUpdate(const json& message)
{
    // Check if "timestamp" is present and not null
    if (!message.contains("timestamp") || message.at("timestamp").is_null())
    {
        std::cerr << "[ERROR] Missing or null timestamp.\n";
        return false;
    }

    // Check if "order_id" is present and not null
    if (!message.contains("hub_id") || message.at("hub_id").is_null())
    {
        std::cerr << "[ERROR] Missing or null hub_id.\n";
        return false;
    }

    // Check if "order_id" is present and not null
    if (!message.contains("order_id") || message.at("order_id").is_null())
    {
        std::cerr << "[ERROR] Missing or null order_id.\n";
        return false;
    }

    // Check if "status" is present and not null
    if (!message.contains("status") || message.at("status").is_null())
    {
        std::cerr << "[ERROR] Missing or null status.\n";
        return false;
    }

    return true; // All checks passed, data is valid
}
