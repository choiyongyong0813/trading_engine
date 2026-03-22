#pragma once
#include <unordered_map>

class OrderBook {
public:
    void addOrder(int orderId);
    void cancelOrder(int orderId);

private:
    std::unordered_map<int, bool> orders_;
};