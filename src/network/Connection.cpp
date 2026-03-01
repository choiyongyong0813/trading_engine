#include "Connection.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

/*
 * 생성자
 * reconnectTimer_ 와 heartbeatTimer_는 io_context와 연결됨
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

    // reconnect 이후에는 socket이 닫혀있을 수 있음
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
 * reconnect 로직
 * 일정 시간 후 다시 Connect 시도
 */
void Connection::StartReconnect() {

    connected_ = false;

    if (socket_.is_open()) {
        socket_.close();
    }

    std::cout << "[Reconnect in 3 seconds...]" << std::endl;

    reconnectTimer_.expires_after(std::chrono::seconds(3));

    reconnectTimer_.async_wait(
        [this](auto ec) {
            if (!ec) {
                Connect();
            }
        }
    );
}

/*
 * heartbeat
 * 연결 유지 확인용 주기적 전송
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
 * 외부 메시지 전송
 * [4byte length][body] 형태로 패킷 구성
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
 * 실제 async_write 수행
 * 동시에 하나의 write만 허용
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
 * 메시지 처리
 * 현재는 단순 출력
 * 추후 protocol/Parser로 분리 예정
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