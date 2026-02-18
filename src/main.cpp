#include "network/Connection.h"
#include <boost/asio.hpp>

int main() {

    // Boost.Asio의 이벤트 루프
    boost::asio::io_context io;

    // example.com:80 → HTTP 테스트 목적
    Connection conn(io, "example.com", 80);

    // 서버 접속 시작
    conn.Connect();

    // 이벤트 루프 실행 (connect/read 콜백이 여기서 처리됨)
    io.run();

    return 0;
}