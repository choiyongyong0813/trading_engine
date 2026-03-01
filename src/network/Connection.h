#pragma once

#include <boost/asio.hpp>
#include <string>
#include <array>
#include <vector>
#include <deque>

class Connection {
public:
    Connection(boost::asio::io_context& io,
               const std::string& host,
               int port);

    void Connect();
    void Close();
    void Send(const std::string& message);

private:
    // 수신
    void ReadHeader();
    void ReadBody(std::size_t bodyLength);
    void ProcessMessage();

    // 송신
    void DoWrite();

    // 안정성 관련
    void StartReconnect();
    void StartHeartbeat();

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;

    boost::asio::steady_timer reconnectTimer_;
    boost::asio::steady_timer heartbeatTimer_;

    std::string host_;
    int port_;

    bool connected_ = false;
    // 재접속 고도화 관련 변수
    bool reconnecting_ = false;   // 중복 방지
    int reconnectDelay_ = 1;      // 시작 1초
    const int maxReconnectDelay_ = 30; // 최대 30초
    std::array<char, 4> header_;
    std::vector<char> body_;

    std::deque<std::vector<char>> writeQueue_;
    bool writing_ = false;
};