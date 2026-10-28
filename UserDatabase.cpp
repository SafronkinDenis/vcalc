#include "UserDatabase.h"
#include "Errors.h"
#include <fstream>

bool UserDatabase::load(const std::string& filename) {
    std::ifstream f(filename);
    if (!f)
        throw IOError("Не удалось открыть файл базы клиентов: " + filename, true);

    users_.clear();
    std::string line;
    while (std::getline(f, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find(':');
        if (pos == std::string::npos || pos == 0) continue;
        users_[line.substr(0, pos)] = line.substr(pos + 1);
    }
    return true;
}

bool UserDatabase::findUser(const std::string& login, std::string& password) const {
    auto it = users_.find(login);
    if (it == users_.end()) return false;
    password = it->second;
    return true;
}
