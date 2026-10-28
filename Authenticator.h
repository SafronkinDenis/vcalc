#pragma once
#include "UserDatabase.h"
#include <string>

class Authenticator {
public:
    explicit Authenticator(const UserDatabase& db) : db_(db) {}

    bool authenticate(const std::string& login, const std::string& salt16,
                      const std::string& hashHex) const;

    static std::string calculateSHA1(const std::string& data);
private:
    const UserDatabase& db_;
};
