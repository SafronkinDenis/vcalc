#include "Logger.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

Logger::Logger(std::string filename) : filename_(std::move(filename)) {
    std::error_code ec;
    auto parent = std::filesystem::path(filename_).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent, ec);
}

void Logger::write(const std::string& level, const std::string& message) {
    char ts[32];
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
    localtime_r(&t, &tmv);
    std::strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tmv);
    std::string line = std::string(ts) + " | " + level + " | " + message;

    std::ofstream f(filename_, std::ios::app);
    if (f) f << line << '\n';
    else   std::cerr << "[журнал недоступен] " << line << std::endl;
}

void Logger::logError(const std::string& message, bool critical) {
    write(critical ? "CRITICAL" : "ERROR", message);
}

void Logger::log(const std::string& message) { write("INFO", message); }
