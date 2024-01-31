#include <string>
#include <chrono>
#include <iostream>

#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{

    int EXPERIMENTS = 10;
    int ITERATIONS = 10000;
    std::string INSTANCE = "Call_7_Vehicle_3";

    std::default_random_engine rng = std::default_random_engine{std::random_device {}()};

    Problem problem = Parser::parseProblem("data/" + INSTANCE + ".txt");
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;
    double bestObjective = initialObjective;

    std::chrono::steady_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < EXPERIMENTS; i++) {
        Solution bestSolution = Solution::initialSolution(&problem);
        for (int j = 0; j < ITERATIONS; j++) {
            Solution solution = Solution::randomSolution(&problem, rng);

            if (solution.isFeasible() && solution.getCost() < bestSolution.getCost()) {
                bestSolution = solution;
            }
        }
        averageObjective += bestSolution.getCost() / EXPERIMENTS;
        if (bestSolution.getCost() < bestObjective) {
            bestObjective = bestSolution.getCost();
            bestSolutionOverall = bestSolution;
        }
    }
    std::chrono::steady_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();

    double improvement = 100 * (initialObjective - bestObjective) / initialObjective;
    long long runningTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count();

    Debugger::printResults(INSTANCE, averageObjective, bestObjective, improvement, runningTime, &bestSolutionOverall);

    return 0;
}
