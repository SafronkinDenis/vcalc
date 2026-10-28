#include "Communicator.h"
#include "Errors.h"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

Communicator::Communicator(int socket) : socket_(socket) {}
Communicator::~Communicator() { close(); }

void Communicator::close() {
    if (socket_ >= 0) { ::close(socket_); socket_ = -1; }
}

size_t Communicator::receiveSome(void* buf, size_t maxLen) {
    for (;;) {
        ssize_t r = ::recv(socket_, buf, maxLen, 0);
        if (r > 0) return static_cast<size_t>(r);
        if (r == 0) throw NetworkError("Клиент закрыл соединение");
        if (errno == EINTR) continue;
        throw NetworkError(std::string("Ошибка recv: ") + std::strerror(errno));
    }
}

void Communicator::receive(void* buf, size_t len) {
    auto* p = static_cast<char*>(buf);
    while (len > 0) {
        size_t n = receiveSome(p, len);
        p += n; len -= n;
    }
}

void Communicator::send(const void* buf, size_t len) {
    auto* p = static_cast<const char*>(buf);
    while (len > 0) {
        ssize_t r = ::send(socket_, p, len, MSG_NOSIGNAL);
        if (r < 0) {
            if (errno == EINTR) continue;
            throw NetworkError(std::string("Ошибка send: ") + std::strerror(errno));
        }
        p += r; len -= static_cast<size_t>(r);
    }
}
