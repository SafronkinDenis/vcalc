#pragma once
#include <exception>
#include <string>

class ProgramError : public std::exception {
public:
    explicit ProgramError(std::string message, bool critical = false)
        : message_(std::move(message)), critical_(critical) {}
    const char* what() const noexcept override { return message_.c_str(); }

    bool isCritical() const noexcept { return critical_; }
private:
    std::string message_;
    bool critical_;
};

class IOError : public ProgramError { public: using ProgramError::ProgramError; };

class InterfaceError : public ProgramError { public: using ProgramError::ProgramError; };

class NetworkError : public ProgramError { public: using ProgramError::ProgramError; };

class AuthError : public ProgramError { public: using ProgramError::ProgramError; };
