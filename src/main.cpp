#include "network/Connection.h"
#include "network/EchoServer.h"

#include "core/Dispatcher.h"
#include "core/Engine.h"
#include "protocol/Message.h"

#include <boost/asio.hpp>
#include <thread>
#include <chrono>

int main() {

    boost::asio::io_context io;

    // 서버 (테스트용)
    EchoServer server(io, 9000);

    Dispatcher dispatcher;
    Engine engine;

    // 핸들러 등록
    dispatcher.registerHandler(MessageType::ORDER_NEW,
        [&](const Message& msg) { engine.handleOrderNew(msg); });

    dispatcher.registerHandler(MessageType::ORDER_CANCEL,
        [&](const Message& msg) { engine.handleOrderCancel(msg); });

    dispatcher.registerHandler(MessageType::HEARTBEAT,
        [&](const Message& msg) { engine.handleHeartbeat(msg); });

    // dispatcher 주입
    Connection conn(io, "127.0.0.1", 9000, dispatcher);
    conn.Connect();

    // io 별도 스레드
    std::thread t([&]() {
        io.run();
    });

    // 연결 안정화 대기
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 테스트 메시지
    conn.Send("ORDER_NEW|orderId=1");
    conn.Send("ORDER_CANCEL|orderId=1");

    t.join();

    return 0;
}