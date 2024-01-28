#include "solution.h"

Solution Solution::initialSolution(Problem* problem) {
    // Create an empty solution
    Solution solution = Solution();

    // Link problem to solution
    solution.problem = problem;

    // Reserve representation size
    solution.representation.reserve(problem->noVehicles + 2 * problem->noCalls);

    // Outsource all calls
    for (int i = 0; i < problem->noVehicles; i++) {
        solution.representation.push_back(0);
    }
    for (int i = 0; i < problem->noCalls; i++) {
        solution.representation.push_back(i);
        solution.representation.push_back(i);
    }

    // Return initial solution
    return solution
};