#include "testcase.h"

void InstanceRunner::blindRandomSearch(std::string instanceName, int experiments, int iterations, std::default_random_engine& rng) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instanceName + ".txt");
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    // Get the time before starting
    std::chrono::steady_clock::time_point timeStart = std::chrono::high_resolution_clock::now();

    // Run the experiments
    for (int i = 0; i < experiments; i++) {
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

        // At the end of the experiment, count the current best cost towards the average cost
        averageObjective += (double)bestSolution.getCost() / experiments;
        // and check if it is better than the current best overall solution
        if (bestSolution.getCost() < bestSolutionOverall.getCost()) {
            bestSolutionOverall = bestSolution;
        }
    }
    // Get the time after ending
    std::chrono::steady_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();

    // Calculate the running time as the difference between starttime and endtime
    long long runningTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count();

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Print the results to standard output
    Debugger::printResults(instanceName, averageObjective, bestSolutionOverall.getCost(), improvement, runningTime, &bestSolutionOverall);
}