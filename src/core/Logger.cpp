#include "core/Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

void Logger::info(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void Logger::error(const std::string& msg) {
    std::cout << "[ERROR] " << msg << std::endl;
}