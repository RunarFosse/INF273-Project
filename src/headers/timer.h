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

    /**
     * @brief Check the current time since timer was last started.
     * 
     * @return Time in seconds
     */
    double check();

    private:
    double average = 0;
    int iterations = 0;
    int experiments;
    std::string title;
    std::chrono::steady_clock::time_point started;
};