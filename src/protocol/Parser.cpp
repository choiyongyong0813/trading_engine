#include "Parser.h"

/*
 * 단순 문자열 변환
 * 추후 JSON, FIX, Binary 프로토콜 확장 가능
 */
Message Parser::Parse(const std::vector<char>& body) {

    Message msg;
    msg.payload = std::string(body.begin(), body.end());

    return msg;
}