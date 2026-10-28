#include "Authenticator.h"
#include "Errors.h"
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {
inline uint32_t rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

bool isHex(const std::string& s) {
    for (unsigned char c : s) if (!std::isxdigit(c)) return false;
    return true;
}
std::string toUpper(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}
}

std::string Authenticator::calculateSHA1(const std::string& data) {
    uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};

    std::vector<uint8_t> msg(data.begin(), data.end());
    const uint64_t bitLen = static_cast<uint64_t>(data.size()) * 8;
    msg.push_back(0x80);
    while (msg.size() % 64 != 56) msg.push_back(0);
    for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>(bitLen >> (i * 8)));

    for (size_t off = 0; off < msg.size(); off += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i)
            w[i] = (uint32_t(msg[off + 4*i]) << 24) | (uint32_t(msg[off + 4*i + 1]) << 16) |
                   (uint32_t(msg[off + 4*i + 2]) << 8) |  uint32_t(msg[off + 4*i + 3]);
        for (int i = 16; i < 80; ++i)
            w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if      (i < 20) { f = (b & c) | (~b & d);          k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;                   k = 0xCA62C1D6; }
            uint32_t tmp = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(b, 30); b = a; a = tmp;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
    }

    char buf[41];
    for (int i = 0; i < 5; ++i) std::snprintf(buf + 8*i, 9, "%08X", h[i]);
    return std::string(buf, 40);
}

bool Authenticator::authenticate(const std::string& login, const std::string& salt16,
                                 const std::string& hashHex) const {
    std::string password;
    if (!db_.findUser(login, password))
        throw AuthError("Пользователь не найден: " + login);

    if (salt16.size() != 16 || !isHex(salt16) || hashHex.size() != 40 || !isHex(hashHex))
        throw AuthError("Некорректный формат соли или хэша, пользователь: " + login);

    if (calculateSHA1(toUpper(salt16) + password) != toUpper(hashHex))
        throw AuthError("Неверный пароль, пользователь: " + login);
    return true;
}
