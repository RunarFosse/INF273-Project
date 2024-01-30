#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "problem.h"

class Solution {
    public:
    std::vector<int> representation;

    /**
     * @brief Constructs the intial solution for the given problem.
     * The initial solution is the baseline solution, where none of the 
     * given calls are answered by "our" vehicles. All are "outsourced".
     * 
     * @param problem Problem to solve
     * @return Initial solution for the given problem
     */
    static Solution initialSolution(Problem* problem);

    /**
     * @brief Checks if the given solution is feasible.
     * Results are cached to prevent redundant computation.
     * 
     * @return true if feasible,
     * @return false if infeasible
     */
    bool isFeasible();

    /**
     * @brief Calculates and returns the cost of the solution.
     * Results are cached to prevent redundant computation.
     * 
     * @note Solutions are assumed feasible. No feasibility check will be ran.
     * 
     * @return Total cost of the solution
     */
    int getCost();

    /**
     * @brief Construct a new Solution given representation as vector.
     * 
     * @param problem Problem to solve
     * @param representation Given solution representation
     */
    Solution(Problem* problem, std::vector<int> representation);

    private:
    // Only allow static instance creation, unless created from given vector (above)
    Solution(Problem* problem);

    Problem* problem;
    std::pair<bool, bool> feasibilityCache;
    std::pair<bool, int> costCache;
};