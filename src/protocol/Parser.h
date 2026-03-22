#pragma once

#include "Message.h"
#include <vector>

/*
 * Parser
 * raw body 데이터를 Message 객체로 변환
 */
class Parser {
public:
    Message Parse(const std::vector<char>& body);
};