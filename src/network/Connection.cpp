#include "Connection.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

/*
 * 생성자
 */
Connection::Connection(boost::asio::io_context& io,
                       const std::string& host,
                       int port)
    : socket_(io),
      resolver_(io),
      reconnectTimer_(io),
      heartbeatTimer_(io),
      host_(host),
      port_(port)
{
}

/*
 * 서버 접속 시작
 */
void Connection::Connect() {

    // reconnect 이후 socket이 닫혀있으면 새로 생성
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

                std::cout << "[Connected]" << std::endl;

                connected_ = true;

                // 🔥 재접속 성공 → delay 초기화
                reconnectDelay_ = 1;
                reconnecting_ = false;

                ReadHeader();
                StartHeartbeat();
            }
            else {

                std::cout << "Connect error: "
                          << ec.message() << std::endl;

                StartReconnect();
            }
        }
    );
}

/*
 * 고도화된 재접속 로직
 * - 중복 실행 방지
 * - exponential backoff
 */
void Connection::StartReconnect() {

    // 이미 재접속 중이면 무시
    if (reconnecting_) return;

    reconnecting_ = true;
    connected_ = false;

    if (socket_.is_open()) {
        socket_.close();
    }

    std::cout << "[Reconnect in "
              << reconnectDelay_
              << " seconds...]" << std::endl;

    reconnectTimer_.expires_after(
        std::chrono::seconds(reconnectDelay_));

    reconnectTimer_.async_wait(
        [this](auto ec) {

            if (ec) return;

            // delay 2배 증가 (최대 30초)
            reconnectDelay_ =
                std::min(reconnectDelay_ * 2,
                         maxReconnectDelay_);

            reconnecting_ = false;

            Connect();
        }
    );
}

/*
 * heartbeat
 * 연결 유지 확인용
 */
void Connection::StartHeartbeat() {

    heartbeatTimer_.expires_after(std::chrono::seconds(5));

    heartbeatTimer_.async_wait(
        [this](auto ec) {

            if (!ec && connected_) {

                std::cout << "[Heartbeat]" << std::endl;

                Send("HEARTBEAT");

                StartHeartbeat();
            }
        }
    );
}

/*
 * 메시지 전송
 */
void Connection::Send(const std::string& message) {

    if (!connected_) return;

    std::vector<char> packet;

    uint32_t length = htonl(message.size());

    packet.resize(4 + message.size());

    std::memcpy(packet.data(), &length, 4);
    std::memcpy(packet.data() + 4,
                message.data(),
                message.size());

    writeQueue_.push_back(std::move(packet));

    if (!writing_) {
        DoWrite();
    }
}

/*
 * 실제 write 수행
 */
void Connection::DoWrite() {

    if (writeQueue_.empty()) return;

    writing_ = true;

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(writeQueue_.front()),
        [this](auto ec, std::size_t) {

            if (ec) {

                std::cout << "[Write Error] "
                          << ec.message() << std::endl;

                StartReconnect();
                return;
            }

            writeQueue_.pop_front();

            if (!writeQueue_.empty()) {
                DoWrite();
            }
            else {
                writing_ = false;
            }
        }
    );
}

/*
 * 4바이트 헤더 읽기
 */
void Connection::ReadHeader() {

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(header_),
        [this](auto ec, std::size_t) {

            if (ec) {

                std::cout << "[Disconnect] "
                          << ec.message() << std::endl;

                StartReconnect();
                return;
            }

            uint32_t bodyLength;
            std::memcpy(&bodyLength, header_.data(), 4);
            bodyLength = ntohl(bodyLength);

            ReadBody(bodyLength);
        }
    );
}

/*
 * body 읽기
 */
void Connection::ReadBody(std::size_t bodyLength) {

    body_.resize(bodyLength);

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(body_),
        [this](auto ec, std::size_t) {

            if (ec) {

                std::cout << "[Disconnect] "
                          << ec.message() << std::endl;

                StartReconnect();
                return;
            }

            ProcessMessage();
            ReadHeader();
        }
    );
}

/*
 * 수신 메시지 처리
 */
void Connection::ProcessMessage() {

    std::string msg(body_.begin(), body_.end());

    std::cout << "[RECV] " << msg << std::endl;
}

/*
 * 수동 종료
 */
void Connection::Close() {

    connected_ = false;

    if (socket_.is_open()) {
        socket_.close();
    }
}