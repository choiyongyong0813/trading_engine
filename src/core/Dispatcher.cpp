#include "core/Dispatcher.h"

void Dispatcher::registerHandler(MessageType type, Handler handler) {
    handlers_[type] = handler;
}

void Dispatcher::dispatch(const Message& msg) {
    auto it = handlers_.find(msg.type);
    
    if (it != handlers_.end()) {
        it->second(msg); // 등록된 handler 실행
    }
    else {
        // 처리 못하는 메시지
        // 실무에서는 로그 찍어야함
    }
}