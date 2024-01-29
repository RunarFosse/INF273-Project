#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include <iostream>

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
    Problem* problem;

    private:
    // Only allow static instance creation
    Solution(Problem* problemPointer);

    std::pair<bool, bool> feasibilityCache;
};