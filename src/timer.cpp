#include "timer.h"

Timer::Timer(std::string title, int experiments) {
    this->title = title;
    this->experiments = experiments;
}

void Timer::start() {
    Debugger::printProgress(this->title, this->iterations++, experiments);
    this->lastStart = std::chrono::high_resolution_clock::now();
}

void Timer::capture() {
    this->average += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - this->lastStart).count() / (1000.0 * this->experiments);
}

double Timer::retrieve() {
    Debugger::printProgress(this->title, this->iterations, experiments);
    return this->average;
}