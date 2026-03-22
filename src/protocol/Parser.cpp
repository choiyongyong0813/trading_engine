#include "Parser.h"
#include <sstream>

/*
 * 메시지 포맷:
 * TYPE|key=value|key=value
 *
 * 예:
 * ORDER_NEW|orderId=123|price=1000
 */
Message Parser::Parse(const std::vector<char>& body) {

    Message msg;

    // raw 데이터 저장 (디버깅용)
    msg.raw = std::string(body.begin(), body.end());

    std::stringstream ss(msg.raw);
    std::string token;

    // 첫 토큰 = MessageType
    if (std::getline(ss, token, '|')) {
        if (token == "LOGIN") msg.type = MessageType::LOGIN;
        else if (token == "ORDER_NEW") msg.type = MessageType::ORDER_NEW;
        else if (token == "ORDER_CANCEL") msg.type = MessageType::ORDER_CANCEL;
        else if (token == "ORDER_ACK") msg.type = MessageType::ORDER_ACK;
        else if (token == "HEARTBEAT") msg.type = MessageType::HEARTBEAT;
        else msg.type = MessageType::UNKNOWN;
    }

    // 나머지 = key=value 파싱
    while (std::getline(ss, token, '|')) {
        size_t pos = token.find('=');

        if (pos == std::string::npos) continue;

        std::string key = token.substr(0, pos);
        std::string value = token.substr(pos + 1);

        msg.fields[key] = value;
    }

    return msg;
}