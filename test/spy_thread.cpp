#include <algorithm>
#include "spy_thread.h"

std::vector<concurrent::spy_thread *> concurrent::spy_thread::alive_threads;
std::mutex concurrent::spy_thread::alive_threads_mutex;

void concurrent::spy_thread::join() {
    m_underlying_thread.join();
}

bool concurrent::spy_thread::joinable() const {
    return m_underlying_thread.joinable();
}

concurrent::spy_thread &concurrent::spy_thread::operator=(concurrent::spy_thread &&other) noexcept {
    m_underlying_thread = std::move(other.m_underlying_thread);
    return *this;
}

concurrent::spy_thread::~spy_thread() {
    std::lock_guard<std::mutex> lock(alive_threads_mutex);
    alive_threads.erase(std::find(alive_threads.begin(), alive_threads.end(), this));
}
