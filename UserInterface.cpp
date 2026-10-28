#include "UserInterface.h"
#include "Errors.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>

namespace {
std::string expandHome(const std::string& path) {
    if (path.size() >= 2 && path[0] == '~' && path[1] == '/') {
        if (const char* home = std::getenv("HOME")) return std::string(home) + path.substr(1);
    }
    return path;
}
}

void UserInterface::showHelp(const char* progName) const {
    std::cout <<
        "Использование: " << progName << " [параметры]\n"
        "Параметры:\n"
        "  -b <файл>  файл базы клиентов     (по умолчанию ~/burmalda/base.conf)\n"
        "  -l <файл>  файл журнала ошибок    (по умолчанию ~/log/errors.log)\n"
        "  -p <порт>  порт сервера           (по умолчанию 33333)\n"
        "  -h         показать эту справку\n";
}

bool UserInterface::parseArgs(int argc, char* argv[], Params& params) {
    params.databaseFile = expandHome("~/burmalda/base.conf");
    params.logFile      = expandHome("~/log/errors.log");
    params.port         = 33333;

    if (argc == 1) { showHelp(argv[0]); return false; }

    optind = 1;
    opterr = 0;
    int opt;
    while ((opt = getopt(argc, argv, "b:l:p:h")) != -1) {
        switch (opt) {
        case 'b': params.databaseFile = expandHome(optarg); break;
        case 'l': params.logFile      = expandHome(optarg); break;
        case 'p': {
            char* end = nullptr;
            long v = std::strtol(optarg, &end, 10);
            if (*optarg == '\0' || *end != '\0' || v < 1 || v > 65535)
                throw InterfaceError(std::string("Некорректный номер порта: ") + optarg);
            params.port = static_cast<uint16_t>(v);
            break;
        }
        case 'h': showHelp(argv[0]); return false;
        default:
            throw InterfaceError(std::string("Неизвестный параметр или не указано значение: -") +
                                 static_cast<char>(optopt));
        }
    }
    if (optind < argc)
        throw InterfaceError(std::string("Лишний аргумент: ") + argv[optind]);
    return true;
}
