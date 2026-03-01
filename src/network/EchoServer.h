#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <array>

/**
 * Length 기반 Echo 서버
 * [4byte length][body]
 */
class EchoServer {
public:
    EchoServer(boost::asio::io_context& io, int port);

private:
    void StartAccept();

    // 세션 클래스 (클라이언트 1명당 1개)
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(boost::asio::ip::tcp::socket socket);

        void Start();

    private:
        void ReadHeader();
        void ReadBody(std::size_t bodyLength);
        void DoWrite();

        boost::asio::ip::tcp::socket socket_;

        std::array<char, 4> header_;
        std::vector<char> body_;
    };

    boost::asio::ip::tcp::acceptor acceptor_;
};