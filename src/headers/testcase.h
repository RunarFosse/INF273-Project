#pragma once

#include <cmath>
#include <string>
#include <random>
#include <functional>

#include "parser.h"
#include "timer.h"
#include "debug.h"
#include "operator.h"

struct EpisodeInformation {
    Solution solution;
    int greedyCost;
    int actualCost;
    int iterfound;
    double timefound;
    int totalIterations;
} typedef EpisodeInformation;

struct AlgorithmInformation {
    std::string instance;
    std::string algorithm;
    double averageObjective;
    Solution bestSolution;
    double improvement;
    double averageTime;
    std::vector<EpisodeInformation> episodes;
} typedef AlgorithmInformation;

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
     * @param instance Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void blindRandomSearch(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Assignment 3.
     * Runs 'Local Search' on the given test case, outputting the results to standard output.
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instance Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void localSearch(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Assignment 3 and 4.
     * Runs 'Simulated Annealing' on the given test case, outputting the results to standard output.
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instance Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void simulatedAnnealing(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Assignment 5.
     * Runs the 'General Adaptive Metaheuristic', as described on the slides, on the given test case, outputting the results to standard output.
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instance Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param iterations Number of iterations per experiment
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     */
    static void generalAdaptiveMetaheuristic(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title);

    /**
     * @brief Final exam.
     * Runs the 'General Adaptive Metaheuristic', as described on the slides, on the given test case,
     * with a deterministic acceptance criterion, outputting the results to standard output. 
     * 
     * @param neighbourOperator Neighbourhood operator
     * @param instance Name of the test case instance to run
     * @param experiments Number of experiments to run
     * @param time Alloted time to run each experiment (in minutes)
     * @param rng Random number generator (for randomness)
     * @param title Output title in loading bar and result txt
     * 
     * @return Return information about the given algorithm.
     */
    static AlgorithmInformation finalAdaptiveMetaheuristic(Operator* neighbourOperator, std::string instance, int experiments, double time, std::default_random_engine& rng, std::string title);

    private:
    // This is a static class, prevent class creation
    InstanceRunner();
};
