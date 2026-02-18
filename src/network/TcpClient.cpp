#include "TcpClient.h"
#include <iostream>

/**
 * 생성자
 * socket_ : 실제 네트워크 소켓
 * resolver_ : 호스트 이름을 IP로 변환하는 resolver 객체
 */
TcpClient::TcpClient(boost::asio::io_context& io, const std::string& host, int port)
    : socket_(io), resolver_(io), host_(host), port_(port)
{
}

/**
 * 서버 접속
 * async_connect는 비동기 연결 함수
 * 연결 성공하면 람다 콜백 실행
 */
void TcpClient::Connect() {
    // 도메인 → IP 변환 (예: "example.com" → 93.184.216.34)
    auto endpoints = resolver_.resolve(host_, std::to_string(port_));

    // 비동기 connect
    boost::asio::async_connect(
        socket_,
        endpoints,
        [this](auto ec, auto) {
            if (!ec) {
                std::cout << "[Connected]" << std::endl;

                // 연결 성공 후 읽기 시작
                StartRead();
            } else {
                std::cout << "Connect error: " << ec.message() << std::endl;
            }
        }
    );
}

/**
 * 비동기 읽기 시작
 * async_read_some: 데이터가 오면 콜백 호출
 */
void TcpClient::StartRead() {
    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [this](auto ec, auto bytes) {
            OnRead(ec, bytes);
        }
    );
}

/**
 * 읽기 콜백
 * - 수신 데이터 처리
 * - 연결 종료 처리
 */
void TcpClient::OnRead(const boost::system::error_code& ec, std::size_t bytes) {
    if (ec) {
        // 요청 중 소켓이 끊기면 여기로 들어옴
        std::cout << "[Disconnect] " << ec.message() << std::endl;
        return;
    }

    // 읽은 데이터 문자열로 변환
    std::string msg(buffer_.data(), bytes);

    std::cout << "[RECV] " << msg << std::endl;

    // 다시 읽기 반복
    StartRead();
}

/**
 * 소켓 종료 (옵션)
 */
void TcpClient::Close() {
    if (socket_.is_open()) {
        socket_.close();
    }
}