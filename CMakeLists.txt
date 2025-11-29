# CMake 최소 요구 버전
# 3.20
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어 설정
project(TradingEngine LANGUAGES CXX)

# C++ 표준 설정 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 빌드할 실행 파일 지정
# src/main.cpp → 소스 파일 목록
add_executable(trading_engine
   src/main.cpp
)
