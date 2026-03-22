#include "Connection.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

/*
 * 생성자
 */
Connection::Connection(boost::asio::io_context& io,
                       const std::string& host,
                       int port,Dispatcher& dispatcher)
    : socket_(io),
      resolver_(io),
      reconnectTimer_(io),
      heartbeatTimer_(io),
      heartbeatTimeoutTimer_(io),
      host_(host),
      port_(port),
       dispatcher_(dispatcher) 
{
}

/*
 * 상태 전이 함수
 */
void Connection::ChangeState(State newState) {

    state_ = newState;

    switch (state_) {

        case State::DISCONNECTED:
            std::cout << "[State] DISCONNECTED" << std::endl;
            break;

        case State::CONNECTING:
            std::cout << "[State] CONNECTING" << std::endl;
            break;

        case State::CONNECTED:
            std::cout << "[State] CONNECTED" << std::endl;
            break;

        case State::RECONNECTING:
            std::cout << "[State] RECONNECTING" << std::endl;
            break;
    }
}

/*
 * 연결 시작
 */
void Connection::Connect() {

    if (state_ == State::CONNECTING ||
        state_ == State::CONNECTED)
        return;

    ChangeState(State::CONNECTING);

    if (!socket_.is_open()) {
        socket_ = boost::asio::ip::tcp::socket(resolver_.get_executor());
    }

    auto endpoints =
        resolver_.resolve(host_, std::to_string(port_));

    boost::asio::async_connect(
        socket_,
        endpoints,
        [this](auto ec, auto) {

            if (!ec) {

                reconnectDelay_ = 1;

                ChangeState(State::CONNECTED);

                ReadHeader();
                StartHeartbeat();
            }
            else {
                StartReconnect();
            }
        }
    );
}

/*
 * 재접속 로직
 */
void Connection::StartReconnect() {

    if (state_ == State::RECONNECTING)
        return;

    ChangeState(State::RECONNECTING);

    CancelHeartbeatTimeout();

    if (socket_.is_open())
        socket_.close();

    reconnectTimer_.expires_after(
        std::chrono::seconds(reconnectDelay_));

    reconnectTimer_.async_wait(
        [this](auto ec) {

            if (ec) return;

            reconnectDelay_ =
                std::min(reconnectDelay_ * 2,
                         maxReconnectDelay_);

            Connect();
        }
    );
}

/*
 * Heartbeat 전송
 */
void Connection::StartHeartbeat() {

    heartbeatTimer_.expires_after(std::chrono::seconds(5));

    heartbeatTimer_.async_wait(
        [this](auto ec) {

            if (!ec &&
                state_ == State::CONNECTED) {

                Send("HEARTBEAT");
                StartHeartbeatTimeout();
                StartHeartbeat();
            }
        }
    );
}

/*
 * Heartbeat Timeout
 */
void Connection::StartHeartbeatTimeout() {

    heartbeatTimeoutTimer_.expires_after(
        std::chrono::seconds(10));

    heartbeatTimeoutTimer_.async_wait(
        [this](auto ec) {

            if (!ec &&
                state_ == State::CONNECTED) {

                std::cout
                    << "[Heartbeat Timeout]"
                    << std::endl;

                StartReconnect();
            }
        }
    );
}

void Connection::CancelHeartbeatTimeout() {
    heartbeatTimeoutTimer_.cancel();
}

/*
 * 수신 처리
 */
void Connection::ReadHeader() {

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(header_),
        [this](auto ec, std::size_t) {

            if (ec) {
                StartReconnect();
                return;
            }

            uint32_t bodyLength;
            std::memcpy(&bodyLength,
                        header_.data(),
                        4);

            bodyLength = ntohl(bodyLength);

            ReadBody(bodyLength);
        }
    );
}

void Connection::ReadBody(std::size_t bodyLength) {

    body_.resize(bodyLength);

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(body_),
        [this](auto ec, std::size_t) {

            if (ec) {
                StartReconnect();
                return;
            }

            // Parser 호출
            Message msg = parser_.Parse(body_);

            dispatcher_.dispatch(msg);
            CancelHeartbeatTimeout();

            ReadHeader();
        }
    );
}

/*
 * 메시지 처리
 */
void Connection::ProcessMessage() {

    std::string msg(body_.begin(), body_.end());

    std::cout << "[RECV] "
              << msg << std::endl;

    CancelHeartbeatTimeout();
}

/*
 * 송신
 */
void Connection::Send(const std::string& message) {

    if (state_ != State::CONNECTED)
        return;

    std::vector<char> packet;

    uint32_t length = htonl(message.size());

    packet.resize(4 + message.size());

    std::memcpy(packet.data(), &length, 4);
    std::memcpy(packet.data() + 4,
                message.data(),
                message.size());

    writeQueue_.push_back(std::move(packet));

    if (!writing_)
        DoWrite();
}

void Connection::DoWrite() {

    if (writeQueue_.empty()) return;

    writing_ = true;

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(writeQueue_.front()),
        [this](auto ec, std::size_t) {

            if (ec) {
                StartReconnect();
                return;
            }

            writeQueue_.pop_front();

            if (!writeQueue_.empty())
                DoWrite();
            else
                writing_ = false;
        }
    );
}

/*
 * 수동 종료
 */
void Connection::Close() {

    ChangeState(State::DISCONNECTED);

    CancelHeartbeatTimeout();

    if (socket_.is_open())
        socket_.close();
}