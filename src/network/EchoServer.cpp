#include "EchoServer.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

using boost::asio::ip::tcp;

/**
 * 서버 생성자
 */
EchoServer::EchoServer(boost::asio::io_context& io, int port)
    : acceptor_(io)
{
    // endpoint 생성
    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::tcp::v4(), port);

    // 소켓 열기
    acceptor_.open(endpoint.protocol());

    // 포트 재사용 옵션 추가 
    acceptor_.set_option(
        boost::asio::socket_base::reuse_address(true));

    // bind
    acceptor_.bind(endpoint);

    // listen
    acceptor_.listen();

    // accept 시작
    StartAccept();
}
/**
 * 클라이언트 접속 대기
 */
void EchoServer::StartAccept() {

    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {

            if (!ec) {
                std::cout << "[Client Connected]" << std::endl;

                std::make_shared<Session>(std::move(socket))->Start();
            }

            StartAccept();
        }
    );
}

/**
 * 세션 생성자
 */
EchoServer::Session::Session(tcp::socket socket)
    : socket_(std::move(socket))
{
}

/**
 * 세션 시작
 */
void EchoServer::Session::Start() {
    ReadHeader();
}

/**
 * 4바이트 헤더 읽기
 */
void EchoServer::Session::ReadHeader() {

    auto self = shared_from_this();

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(header_),
        [this, self](auto ec, std::size_t) {

            if (ec) {
                std::cout << "[Client Disconnect]" << std::endl;
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
 * body 읽기
 */
void EchoServer::Session::ReadBody(std::size_t bodyLength) {

    body_.resize(bodyLength);

    auto self = shared_from_this();

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(body_),
        [this, self](auto ec, std::size_t) {

            if (ec) {
                std::cout << "[Client Disconnect]" << std::endl;
                return;
            }

            std::string msg(body_.begin(), body_.end());
            std::cout << "[SERVER RECV] " << msg << std::endl;

            // 받은 데이터 그대로 다시 보냄
            DoWrite();
        }
    );
}

/**
 * Echo 전송
 */
void EchoServer::Session::DoWrite() {

    uint32_t length = htonl(body_.size());

    std::vector<char> packet(4 + body_.size());

    std::memcpy(packet.data(), &length, 4);
    std::memcpy(packet.data() + 4, body_.data(), body_.size());

    auto self = shared_from_this();

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(packet),
        [this, self](auto ec, std::size_t) {

            if (ec) {
                return;
            }

            // 다시 다음 메시지 대기
            ReadHeader();
        }
    );
}