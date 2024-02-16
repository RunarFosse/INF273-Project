#include <string>
#include <random>
#include <chrono>

#include "parser.h"
#include "debug.h"

class InstanceRunner {
    public:
    /**
     * @brief Tests a given algorithm on all different testcases. Writes results to stdout and current progress to terminal.
     * 
     * @param algorithm Algorithm to test
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations to run each experiment
     * @param rng Random number generator engine
     */
    static void testAlgorithm(std::function<void(std::string, int, int, std::default_random_engine&)> algorithm, bool shallow, int experiments, int iterations, std::default_random_engine& rng);

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

    /**
     * @brief Assignment 3.
     * Runs 'Local Search' on the given test case, outputting the results to standard output.
     * 
     * @param instanceName Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     */
    static void localSearch(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng);

    /**
     * @brief Assignment 3.
     * Runs 'Simulated Annealing' on the given test case, outputting the results to standard output.
     * 
     * @param instanceName Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     */
    static void simulatedAnnealing(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng);

    private:
    // This is a static class, prevent class creation
    InstanceRunner();
};
