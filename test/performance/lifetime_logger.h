#pragma once

#include <chrono>
#include <string>
#include <iostream>

class lifetime_logger {
private:
    using Clock = std::chrono::high_resolution_clock;
    const std::string mMessage;
    decltype(Clock::now()) mBegin;
    std::ostream &mStream;

public:
    lifetime_logger(std::string message, std::ostream &stream = std::cout):
            mMessage(std::move(message)),
            mBegin(Clock::now()),
            mStream(stream) {

    }

    ~lifetime_logger() noexcept {
        const auto lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - mBegin).count();
        std::cout << mMessage << lifetime << " ms" << std::endl;
    }

};