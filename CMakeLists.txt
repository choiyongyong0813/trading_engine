# CMake 최소 요구 버전
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어
project(TradingEngine LANGUAGES CXX)

# C++17 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost 라이브러리 검색 (Asio는 system 모듈 필요)
find_package(Boost REQUIRED COMPONENTS system)

# Boost 헤더 포함
include_directories(${Boost_INCLUDE_DIRS})

# 실행 파일에 포함할 소스 파일들
add_executable(trading_engine
    src/main.cpp
    src/network/Connection.cpp
    src/network/EchoServer.cpp
    src/protocol/Parser.cpp
)

# Boost 라이브러리 링크
target_link_libraries(trading_engine
    ${Boost_LIBRARIES}
)
