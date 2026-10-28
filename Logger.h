#pragma once
#include <string>

class Logger {
public:

    explicit Logger(std::string filename);

    void logError(const std::string& message, bool critical = false);

    void log(const std::string& message);
private:
    void write(const std::string& level, const std::string& message);
    std::string filename_;
};
