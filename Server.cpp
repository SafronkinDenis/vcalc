#include "Server.h"
#include "Communicator.h"
#include "Errors.h"
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

namespace {
volatile sig_atomic_t g_stop = 0;
void onSignal(int) { g_stop = 1; }

constexpr size_t   kSaltLen       = 16;
constexpr size_t   kHashLen       = 40;
constexpr size_t   kMaxAuthMsg    = 512;
constexpr uint32_t kMaxVectorSize = 16u * 1024 * 1024;
constexpr int      kClientTimeout = 30;
}

Server::Server(const Params& params, Logger& log)
    : params_(params), log_(log), auth_(db_) {}

Server::~Server() {
    if (listenFd_ >= 0) ::close(listenFd_);
}

void Server::start() {
    db_.load(params_.databaseFile);

    listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0)
        throw NetworkError(std::string("socket: ") + std::strerror(errno), true);

    int yes = 1;
    ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(params_.port);
    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0)
        throw NetworkError(std::string("bind: ") + std::strerror(errno), true);
    if (::listen(listenFd_, 16) < 0)
        throw NetworkError(std::string("listen: ") + std::strerror(errno), true);

    struct sigaction sa{};
    sa.sa_handler = onSignal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    std::signal(SIGPIPE, SIG_IGN);

    log_.log("Сервер запущен, порт " + std::to_string(params_.port));

    while (running_ && !g_stop) {
        int fd = acceptClient();
        if (fd >= 0) handleClient(fd);
    }
    log_.log("Сервер остановлен");
}

int Server::acceptClient() {
    sockaddr_in ca{};
    socklen_t len = sizeof ca;
    int fd = ::accept(listenFd_, reinterpret_cast<sockaddr*>(&ca), &len);
    if (fd < 0) {
        if (errno != EINTR)
            log_.logError(std::string("accept: ") + std::strerror(errno), false);
        return -1;
    }
    timeval tv{kClientTimeout, 0};
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
    return fd;
}

void Server::handleClient(int fd) {
    Communicator comm(fd);
    try {

        char buf[kMaxAuthMsg];
        std::string msg(buf, comm.receiveSome(buf, sizeof buf));
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r' || msg.back() == '\0'))
            msg.pop_back();

        try {
            if (msg.size() <= kSaltLen + kHashLen)
                throw AuthError("Некорректное сообщение аутентификации");
            std::string hash  = msg.substr(msg.size() - kHashLen);
            std::string salt  = msg.substr(msg.size() - kHashLen - kSaltLen, kSaltLen);
            std::string login = msg.substr(0, msg.size() - kHashLen - kSaltLen);
            auth_.authenticate(login, salt, hash);
            log_.log("Успешный вход: " + login);
        } catch (const AuthError& e) {
            log_.logError(e.what(), false);
            comm.send(std::string("ERR"));
            return;
        }
        comm.send(std::string("OK"));

        uint32_t count = 0;
        comm.receive(&count, sizeof count);

        std::vector<uint32_t> results;
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t size = 0;
            comm.receive(&size, sizeof size);
            if (size > kMaxVectorSize)
                throw ProgramError("Слишком большой размер вектора: " + std::to_string(size));
            std::vector<uint32_t> v(size);
            if (size) comm.receive(v.data(), static_cast<size_t>(size) * sizeof(uint32_t));
            results.push_back(processor_.calculateSumOfSquares(v));
        }

        comm.send(&count, sizeof count);
        if (!results.empty())
            comm.send(results.data(), results.size() * sizeof(uint32_t));

        log_.log("Клиент отключен");
    } catch (const ProgramError& e) {
        log_.logError(e.what(), false);
    } catch (const std::exception& e) {
        log_.logError(std::string("Ошибка обработки клиента: ") + e.what(), false);
    }
}
