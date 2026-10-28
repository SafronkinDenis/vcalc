#pragma once
#include "Authenticator.h"
#include "DataProcessor.h"
#include "Logger.h"
#include "UserDatabase.h"
#include "UserInterface.h"

class Server {
public:
    Server(const Params& params, Logger& log);
    ~Server();

    void start();
    void stop() { running_ = false; }
private:
    int  acceptClient();
    void handleClient(int fd);

    Params        params_;
    Logger&       log_;
    UserDatabase  db_;
    Authenticator auth_;
    DataProcessor processor_;
    int           listenFd_ = -1;
    bool          running_  = true;
};
