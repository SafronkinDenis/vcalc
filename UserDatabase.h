#pragma once
#include <map>
#include <string>

class UserDatabase {
public:

    bool load(const std::string& filename);

    bool findUser(const std::string& login, std::string& password) const;
private:
    std::map<std::string, std::string> users_;
};
