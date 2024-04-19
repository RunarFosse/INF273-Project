#include "timer.h"

Timer::Timer(std::string title, int experiments) {
    this->title = title;
    this->experiments = experiments;
}

void Timer::start() {
    Debugger::printProgress(this->title, this->iterations++, experiments);
    this->started = std::chrono::high_resolution_clock::now();
}

void Timer::capture() {
    std::chrono::steady_clock::time_point ended = std::chrono::high_resolution_clock::now();
    this->average += std::chrono::duration_cast<std::chrono::milliseconds>(ended - this->started).count() / (1000.0 * this->experiments);
}

double Timer::retrieve() {
    Debugger::printProgress(this->title, this->iterations, experiments);
    return this->average;
}

 double Timer::check() {
    std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - this->started).count() / (1000.0);
 }