#include "solution.h"

Solution::Solution(Problem* problem) {
    // Link problem to solution
    problem = problem;

    // Reserve representation size
    representation.reserve(problem->noVehicles + 2 * problem->noCalls);
}

Solution Solution::initialSolution(Problem* problem) {
    // Create an empty solution
    Solution solution(problem);

    // Outsource all calls
    for (int i = 0; i < problem->noVehicles; i++) {
        solution.representation.push_back(0);
    }
    for (int i = 1; i <= problem->noCalls; i++) {
        solution.representation.push_back(i);
        solution.representation.push_back(i);
    }

    // Return initial solution
    return solution;
}