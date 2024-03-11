#pragma once

#include <cmath>
#include <string>
#include <random>

#include "parser.h"
#include "timer.h"
#include "debug.h"
#include "operator.h"

class InstanceRunner {
    public:
    /**
     * @brief Tests a given algorithm on all different testcases. Writes results to stdout and current progress to terminal.
     * 
     * @param algorithm Algorithm to test
     * @param neighbourOperator Neighbourhood operator
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations to run each experiment
     * @param rng Random number generator engine
     * @param title Output title in loading bar and result txt
     */
    static void testAlgorithm(std::function<void(Operator*, std::string, int, int, std::default_random_engine&, std::string)> algorithm, Operator* neighbourOperator, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Assignment 2. 
     * Runs 'Blind Random Search' on the given test case, outputting the results to standard output.
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instanceName Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void blindRandomSearch(Operator* neighbourOperator, std::string instanceName, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Assignment 3.
     * Runs 'Local Search' on the given test case, outputting the results to standard output.
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instanceName Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void localSearch(Operator* neighbourOperator, std::string instanceName, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Assignment 3.
     * Runs 'Simulated Annealing' on the given test case, outputting the results to standard output.
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instanceName Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void simulatedAnnealing(Operator* neighbourOperator, std::string instanceName, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    private:
    // This is a static class, prevent class creation
    InstanceRunner();
};
