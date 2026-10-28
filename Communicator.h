#pragma once
#include <cstddef>
#include <string>

class Communicator {
public:
    explicit Communicator(int socket);
    ~Communicator();
    Communicator(const Communicator&) = delete;
    Communicator& operator=(const Communicator&) = delete;

    size_t receiveSome(void* buf, size_t maxLen);

    void receive(void* buf, size_t len);

    void send(const void* buf, size_t len);
    void send(const std::string& s) { send(s.data(), s.size()); }
    void close();
private:
    int socket_;
};
