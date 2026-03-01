#include "Connection.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>   // htonl, ntohl

/**
 * 생성자
 */
Connection::Connection(boost::asio::io_context& io,
                       const std::string& host,
                       int port)
    : socket_(io),
      resolver_(io),
      host_(host),
      port_(port)
{
}

/**
 * 서버 접속
 */
void Connection::Connect() {

    auto endpoints =
        resolver_.resolve(host_, std::to_string(port_));

    boost::asio::async_connect(
        socket_,
        endpoints,
        [this](auto ec, auto) {
            if (!ec) {
                std::cout << "[Connected]" << std::endl;
                ReadHeader();
            } else {
                std::cout << "Connect error: "
                          << ec.message() << std::endl;
            }
        }
    );
}

/**
 * 외부에서 메시지 전송 요청
 * 1. length 붙여서 패킷 생성
 * 2. queue에 push
 * 3. write 진행 중 아니면 DoWrite 시작
 */
void Connection::Send(const std::string& message) {

    // body를 vector로 복사
    std::vector<char> packet;

    uint32_t length = htonl(message.size());

    // 4바이트 length 추가
    packet.resize(4 + message.size());
    std::memcpy(packet.data(), &length, 4);
    std::memcpy(packet.data() + 4,
                message.data(),
                message.size());

    // queue에 추가
    writeQueue_.push_back(std::move(packet));

    // 현재 write 중이 아니면 시작
    if (!writing_) {
        DoWrite();
    }
}

/**
 * 실제 async_write 실행 함수
 * 항상 하나의 write만 수행
 */
void Connection::DoWrite() {

    writing_ = true;

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(writeQueue_.front()),
        [this](auto ec, std::size_t) {

            if (ec) {
                std::cout << "[Write Error] "
                          << ec.message() << std::endl;
                return;
            }

            // 현재 메시지 제거
            writeQueue_.pop_front();

            // 다음 메시지 있으면 계속 write
            if (!writeQueue_.empty()) {
                DoWrite();
            }
            else {
                writing_ = false;
            }
        }
    );
}

/**
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
                return;
            }

            uint32_t bodyLength;
            std::memcpy(&bodyLength, header_.data(), 4);
            bodyLength = ntohl(bodyLength);

            ReadBody(bodyLength);
        }
    );
}

/**
 * bodyLength 만큼 정확히 읽기
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
                return;
            }

            ProcessMessage();
            ReadHeader();
        }
    );
}

/**
 * 수신 메시지 처리
 */
void Connection::ProcessMessage() {

    std::string msg(body_.begin(), body_.end());

    std::cout << "[RECV] " << msg << std::endl;
}

/**
 * 소켓 종료
 */
void Connection::Close() {
    if (socket_.is_open()) {
        socket_.close();
    }
}