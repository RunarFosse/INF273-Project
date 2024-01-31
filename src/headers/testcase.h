#include <string>
#include <random>
#include <chrono>

#include "parser.h"
#include "debug.h"

class InstanceRunner {
    public:
    /**
     * @brief Assignment 2. 
     * Runs 'Blind Random Search' on the given test case, outputting the results to standard output.
     * 
     * @param instanceName Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     */
    static void blindRandomSearch(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng);

    private:
    // This is a static class, prevent class creation
    InstanceRunner();
};
