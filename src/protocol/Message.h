#pragma once
#include <string>
#include <unordered_map>

// 메시지 타입 enum
enum class MessageType {
    LOGIN,
    ORDER_NEW,
    ORDER_CANCEL,
    ORDER_ACK,
    HEARTBEAT,
    UNKNOWN
};

// 메시지 구조체
struct Message {
    MessageType type;   // 메시지 타입
    std::string raw;    // 원본 데이터 (로그/디버깅용)
    
    // key=value 형태 필드 저장 (확장성 핵심)
    std::unordered_map<std::string, std::string> fields;
};