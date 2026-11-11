#include "UserInterface.h"
#include "Errors.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace {
std::string expandHome(const std::string& path) {
    if (path.size() >= 2 && path[0] == '~' && path[1] == '/') {
        if (const char* home = std::getenv("HOME"))
            return std::string(home) + path.substr(1);
    }
    return path;
}

// Проверка, что строка — это корректный порт (число от 1 до 65535)
bool isValidPort(const std::string& s, uint16_t& port) {
    if (s.empty()) return false;
    for (char c : s)
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    long v = std::strtol(s.c_str(), nullptr, 10);
    if (v < 1 || v > 65535) return false;
    port = static_cast<uint16_t>(v);
    return true;
}
}

void UserInterface::showHelp(const char* progName) const {
    std::cout <<
        "Использование: " << progName << " -r|--run [параметры]\n"
        "Параметры:\n"
        "  -r, --run    запустить сервер\n"
        "  -b <файл>    файл базы клиентов     (по умолчанию base.conf)\n"
        "  -l <файл>    файл журнала ошибок    (по умолчанию errors.log)\n"
        "  -p <порт>    порт сервера           (по умолчанию 33333)\n"
        "  -h, --help   показать эту справку\n";
}

bool UserInterface::parseArgs(int argc, char* argv[], Params& params) {
    // Значения по умолчанию
    params.databaseFile = "base.conf";
    params.logFile      = "errors.log";
    params.port         = 33333;
    params.run          = false;

    // Нет параметров — показываем справку
    if (argc <= 1) {
        showHelp(argv[0]);
        return false;
    }

    // Ручной разбор аргументов
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            showHelp(argv[0]);
            return false;
        }

        if (arg == "-r" || arg == "--run") {
            params.run = true;
            continue;
        }

        if (arg == "-b" || arg == "--database") {
            if (i + 1 >= argc)
                throw InterfaceError("Не указано значение для параметра: " + arg);
            params.databaseFile = expandHome(argv[++i]);
            continue;
        }

        if (arg == "-l" || arg == "--log") {
            if (i + 1 >= argc)
                throw InterfaceError("Не указано значение для параметра: " + arg);
            params.logFile = expandHome(argv[++i]);
            continue;
        }

        if (arg == "-p" || arg == "--port") {
            if (i + 1 >= argc)
                throw InterfaceError("Не указано значение для параметра: " + arg);
            uint16_t p;
            if (!isValidPort(argv[++i], p))
                throw InterfaceError(std::string("Некорректный номер порта: ") + argv[i]);
            params.port = p;
            continue;
        }

        // Неизвестный параметр
        throw InterfaceError(std::string("Неизвестный параметр: ") + arg);
    }

    // Если -r/--run не указан — показываем справку
    if (!params.run) {
        std::cout << "Ошибка: не указан режим запуска. Используйте -r или --run\n\n";
        showHelp(argv[0]);
        return false;
    }

    return true;
}
