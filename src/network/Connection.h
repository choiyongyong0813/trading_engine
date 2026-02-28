#pragma once

#include <boost/asio.hpp>
#include <string>
#include <array>

/**
 * 비동기 TCP Connection 클래스
 * - 서버에 접속(connect)
 * - 비동기 read (async_read_some)
 * - 연결 종료 처리
 */
class Connection {
public:
    // 생성자: io_context, 호스트명, 포트 전달
    Connection(boost::asio::io_context& io,
               const std::string& host,
               int port);

    // 서버 접속 시작
    void Connect();

    // 소켓 종료
    void Close();

private:
    // 헤더 4바이트 읽기
    void ReadHeader();

    // 바디 읽기
    void ReadBody(std::size_t bodyLength);

    // 메시지 처리
    void ProcessMessage();

private:
    boost::asio::ip::tcp::socket socket_;     // TCP 소켓
    boost::asio::ip::tcp::resolver resolver_; // DNS(도메인 → IP) 해결기

    std::string host_;
    int port_;
    std::array<char, 4> header_;  // length header
    std::vector<char> body_;      // message body

};