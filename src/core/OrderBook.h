#pragma once
#include <unordered_map>

// 단순 주문 저장 구조
class OrderBook {
public:
    void addOrder(int orderId);
    void cancelOrder(int orderId);

private:
    std::unordered_map<int, bool> orders_;
};