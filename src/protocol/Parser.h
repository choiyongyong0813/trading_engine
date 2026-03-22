#pragma once

#include <vector>
#include "Message.h"

// 문자열 → Message 구조 변환 클래스
class Parser {
public:
    // body 데이터를 파싱해서 Message로 변환
    Message Parse(const std::vector<char>& body);
};