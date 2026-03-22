#pragma once
#include <functional>
#include <unordered_map>
#include "protocol/Message.h"

// 메시지 분기 처리 담당
class Dispatcher {
public:
    using Handler = std::function<void(const Message&)>;

    // 메시지 타입별 핸들러 등록
    void registerHandler(MessageType type, Handler handler);

    // 메시지 들어오면 적절한 핸들러 호출
    void dispatch(const Message& msg);

private:
    std::unordered_map<MessageType, Handler> handlers_;
};