#include "testcase.h"

void InstanceRunner::blindRandomSearch(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng) {
    Debugger::printToTerminal("Running Testcase: " + instanceName);

    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instanceName + ".txt");
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    // Keep track of average running time per experiment
    double runningTime = 0;

    // Run the experiments
    for (int i = 0; i < experiments; i++) {
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

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Print the results to standard output
    Debugger::printResults(instanceName, averageObjective, bestSolutionOverall.getCost(), improvement, runningTime, &bestSolutionOverall);
}