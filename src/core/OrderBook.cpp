#include "core/OrderBook.h"
#include "core/Logger.h"

void OrderBook::addOrder(int orderId) {
    orders_[orderId] = true;
    Logger::info("Order added: " + std::to_string(orderId));
}

void OrderBook::cancelOrder(int orderId) {
    orders_.erase(orderId);
    Logger::info("Order canceled: " + std::to_string(orderId));
}