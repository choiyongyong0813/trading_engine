#pragma once
#include "protocol/Message.h"
#include "core/OrderBook.h"

// 모든 비즈니스 로직 담당
class Engine {
public:
    void onMessage(const Message& msg);

    // Dispatcher에서 직접 호출할 핸들러
    void handleLogin(const Message& msg);
    void handleOrderNew(const Message& msg);
    void handleOrderCancel(const Message& msg);
    void handleHeartbeat(const Message& msg);

private:
    OrderBook orderBook_; // 주문 관리
};