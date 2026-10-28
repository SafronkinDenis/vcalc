#include "Errors.h"
#include "Logger.h"
#include "Server.h"
#include "UserInterface.h"
#include <iostream>

int main(int argc, char* argv[]) {
    UserInterface ui;
    Params params;
    try {
        if (!ui.parseArgs(argc, argv, params)) return 0;
    } catch (const InterfaceError& e) {
        std::cerr << "Ошибка: " << e.what() << "\n\n";
        ui.showHelp(argv[0]);
        return 2;
    }

    Logger logger(params.logFile);
    try {
        Server server(params, logger);
        server.start();
    } catch (const ProgramError& e) {
        logger.logError(e.what(), true);
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        logger.logError(e.what(), true);
        return 1;
    }
    return 0;
}
