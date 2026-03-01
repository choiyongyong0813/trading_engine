#pragma once

#include <boost/asio.hpp>
#include <string>
#include <array>
#include <vector>
#include <deque>

/**
 * Length 기반 TCP Connection
 * [4byte length][body] 구조
 * WriteQueue 포함
 */
class Connection {
public:
    Connection(boost::asio::io_context& io,
               const std::string& host,
               int port);

    void Connect();
    void Close();

    /**
     * 외부에서 메시지 전송 요청
     * 문자열을 length 기반 메시지로 만들어 queue에 추가
     */
    void Send(const std::string& message);

private:
    // 수신 관련
    void ReadHeader();
    void ReadBody(std::size_t bodyLength);
    void ProcessMessage();

    // 송신 관련
    void DoWrite();

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;

    std::string host_;
    int port_;

    // 수신 버퍼
    std::array<char, 4> header_;
    std::vector<char> body_;

    // 🔥 WriteQueue
    std::deque<std::vector<char>> writeQueue_;
    bool writing_ = false;
};