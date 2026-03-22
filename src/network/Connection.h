#pragma once

#include <boost/asio.hpp>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include "../protocol/Parser.h"
#include "../core/Dispatcher.h"

/*
 * 운영급 Connection
 * - Length 프레이밍
 * - WriteQueue
 * - Backoff Reconnect
 * - Heartbeat
 * - Heartbeat Timeout
 * - 상태 머신 기반 관리
 */
class Connection {
public:
    Connection(boost::asio::io_context& io,
               const std::string& host,
               int port,
               Dispatcher& dispatcher);

    void Connect();
    void Close();
    void Send(const std::string& message);

private:
    // 상태 정의
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        RECONNECTING
    };
    Parser parser_;
    void ChangeState(State newState);

    // 수신
    void ReadHeader();
    void ReadBody(std::size_t bodyLength);
    void ProcessMessage();

    // 송신
    void DoWrite();

    // 안정성
    void StartReconnect();
    void StartHeartbeat();
    void StartHeartbeatTimeout();
    void CancelHeartbeatTimeout();

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
Dispatcher& dispatcher_; 
    boost::asio::steady_timer reconnectTimer_;
    boost::asio::steady_timer heartbeatTimer_;
    boost::asio::steady_timer heartbeatTimeoutTimer_;

    std::string host_;
    int port_;

    State state_ = State::DISCONNECTED;

    int reconnectDelay_ = 1;
    const int maxReconnectDelay_ = 30;

    std::array<char, 4> header_;
    std::vector<char> body_;

    std::deque<std::vector<char>> writeQueue_;
    bool writing_ = false;
};