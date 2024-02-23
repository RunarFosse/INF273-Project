#pragma once

#include <chrono>
#include <string>

#include "debug.h"

class Timer {
    public:

    /**
     * @brief Construct a new Timer, outputting progress to terminal.
     * 
     * @param title Title of progress bar
     * @param experiments Number of experiments to average time over
     */
    Timer(std::string title, int experiments);

    /**
     * @brief Start another capture iteration.
     * 
     */
    void start();

    /**
     * @brief Finish another capture iteration.
     * 
     */
    void capture();

    /**
     * @brief Retrieve current timed average.
     * 
     * @return Total captured time / number of experiments
     */
    double retrieve();


    private:
    int iterations = 0;
    int experiments;
    std::string title;
    double average;
    std::chrono::steady_clock::time_point lastStart;
};