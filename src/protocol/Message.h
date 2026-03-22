#pragma once
#include <string>

/*
 * 수신 메시지 구조체
 * 현재는 단순 문자열 기반
 * 추후 주문/응답 구조로 확장 가능
 */
struct Message {
    std::string payload;
};