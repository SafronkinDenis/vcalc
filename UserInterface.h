#pragma once
#include <cstdint>
#include <string>

struct Params {
    std::string databaseFile;
    std::string logFile;
    uint16_t    port = 33333;
};

class UserInterface {
public:

    bool parseArgs(int argc, char* argv[], Params& params);
    void showHelp(const char* progName) const;
};
