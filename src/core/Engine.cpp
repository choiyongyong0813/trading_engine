#include "core/Engine.h"
#include "core/Logger.h"

void Engine::onMessage(const Message& msg) {
    // 이건 선택 (Dispatcher 쓰면 안 써도 됨)
}

void Engine::handleLogin(const Message& msg) {
    Logger::info("LOGIN received");
}

void Engine::handleOrderNew(const Message& msg) {
    Logger::info("ORDER_NEW received");

    // 예시 필드 사용
    int orderId = std::stoi(msg.fields.at("orderId"));

    orderBook_.addOrder(orderId);
}

void Engine::handleOrderCancel(const Message& msg) {
    Logger::info("ORDER_CANCEL received");

    int orderId = std::stoi(msg.fields.at("orderId"));
    orderBook_.cancelOrder(orderId);
}

void Engine::handleHeartbeat(const Message& msg) {
    Logger::info("HEARTBEAT received");
}