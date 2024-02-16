#include "testcase.h"

void InstanceRunner::testAlgorithm(std::function<void(std::string, int, int, std::default_random_engine&)> algorithm, int experiments, int iterations, std::default_random_engine& rng) {

    // Redirect standard output to 'results.txt'
    Debugger::outputToFile("results.txt");

    // Hide cursor from terminal
    Debugger::displayCursor(false);
    
    // Run the given algorithm on all different testcases, given experiment and iteration count
    algorithm("Call_7_Vehicle_3", experiments, iterations, rng);
    algorithm("Call_18_Vehicle_5", experiments, iterations, rng);
    algorithm("Call_35_Vehicle_7", experiments, iterations, rng);
    algorithm("Call_80_Vehicle_20", experiments, iterations, rng);
    algorithm("Call_130_Vehicle_40", experiments, iterations, rng);
    algorithm("Call_300_Vehicle_90", experiments, iterations, rng);

    // Show cursor in terminal
    Debugger::displayCursor(true);
}

void InstanceRunner::blindRandomSearch(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instanceName + ".txt");
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    // Keep track of average running time per experiment
    double runningTime = 0;

    // Run the experiments
    for (int i = 0; i < experiments; i++) {
        // Print progress bar
        Debugger::printProgress("Running Testcase: " + instanceName, i, experiments);

        // Get the time before starting current experiment
        std::chrono::steady_clock::time_point timeStart = std::chrono::high_resolution_clock::now();

        // Initialize the initial solution as the "current best"
        Solution bestSolution = Solution::initialSolution(&problem);
        // Run iterations per experiment
        for (int j = 0; j < iterations; j++) {
            // Generate a new random solution
            Solution solution = Solution::randomSolution(&problem, rng);

            // Check if the solution is feasible and, if so, check if it is better than the current best
            if (solution.isFeasible() && solution.getCost() < bestSolution.getCost()) {
                bestSolution = solution;
            }
        }

        // Get the time after ending current experiment
        std::chrono::steady_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();

        // Calculate the running time as the difference between starttime and endtime (in seconds), and add to average running time
        runningTime += std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count() / (1000.0 * experiments);

        // At the end of the experiment, count the current best cost towards the average cost
        averageObjective += (double)bestSolution.getCost() / experiments;
        // and check if it is better than the current best overall solution
        if (bestSolution.getCost() < bestSolutionOverall.getCost()) {
            bestSolutionOverall = bestSolution;
        }
    }

    // Fill the progress bar
    Debugger::printProgress("Running Testcase: " + instanceName, experiments, experiments);

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Print the results to standard output
    Debugger::printResults(instanceName, averageObjective, bestSolutionOverall.getCost(), improvement, runningTime, &bestSolutionOverall);
}

void InstanceRunner::localSearch(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng) {
    // TODO
}

void InstanceRunner::simulatedAnnealing(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng) {
    // TODO
}