#include "Connection.h"
#include <iostream>
#include <cstring>   // memcpy
#include <arpa/inet.h> // ntohl

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

                // 연결 성공 → 헤더 읽기 시작
                ReadHeader();
            }
            else {
                std::cout << "Connect error: "
                          << ec.message() << std::endl;
            }
        }
    );
}

/**
 * 4바이트 헤더 읽기
 * 반드시 정확히 4바이트 읽는다.
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

            // 네트워크 바이트 순서 → host 순서 변환
            uint32_t bodyLength;
            std::memcpy(&bodyLength, header_.data(), 4);
            bodyLength = ntohl(bodyLength);

            // 바디 읽기 시작
            ReadBody(bodyLength);
        }
    );
}

/**
 * bodyLength 만큼 정확히 읽는다.
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

            // 메시지 처리
            ProcessMessage();

            // 다음 메시지 위해 다시 헤더 읽기
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