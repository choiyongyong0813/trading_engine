#include "network/Connection.h"
#include "network/EchoServer.h"
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

int main() {

    boost::asio::io_context io;

    // 서버 실행
    EchoServer server(io, 9000);

    // 클라이언트 생성
    Connection conn(io, "127.0.0.1", 9000);
    conn.Connect();

    // io_context 별도 스레드 실행
    std::thread t([&io]() {
        io.run();
    });

    // 연결 대기
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 메시지 전송 테스트
    conn.Send("HELLO_ENGINE");
    conn.Send("ORDER_REQUEST_123");
    conn.Send("PING");

    t.join();

    return 0;
}