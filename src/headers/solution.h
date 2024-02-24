#pragma once

#include <queue>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>

#include "problem.h"

class Solution {
    public:

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
     * @brief Constructs a random solution, spanning the whole solution space.
     * 
     * @param problem Problem to solve
     * @param rng Random number generator Engine
     * @return Random solution for the given problem 
     */
    static Solution randomSolution(Problem* problem, std::default_random_engine& rng);

    /**
     * @brief Moves a call from wherever it currently is to index1 and index2.
     * 
     * @param callIndex Call to move
     * @param index1 One position
     * @param index2 Other position
     */
    void move(int callIndex, int index1, int index2);

    /**
     * @brief Greedily moves a call from wherever it currently is wihin to [from, to), index1 and index2.
     * 
     * @note Current occurence of call, index1 and index2 is assumed to be located within [from, to).
     * 
     * @param callIndex Call to move
     * @param from Start index (inclusive)
     * @param to End index (inclusive)
     * @param index1 One position
     * @param index2 Other position
     */
    void greedyMove(int callIndex, int from, int to, int index1, int index2);

    /**
     * @brief Outsource a given call. Everything handled automatically.
     * 
     * @param callIndex Call to outsource
     * @return Resulting indices where call was inserted in solution representation
     */
    std::pair<int, int> outsource(int callIndex);

    /**
     * @brief Return the vehicle currently containing callIndex.
     * 
     * @note Solution is assumed to be feasible.
     * 
     * @param callIndex Call of which to find current vehicle of
     * @return Vehicle index
     */
    int getVehicleWith(int callIndex);

    /**
     * @brief Checks if the given solution is feasible.
     * Results are cached to prevent redundant computation.
     * 
     * @note Solutions are assumed valid. Validity checks will NOT be done.
     * 
     * @return true if feasible,
     * @return false if infeasible
     */
    bool isFeasible();

    /**
     * @brief Calculates feasibility of the solution, given that ONLY a specific vehicleIndex changed.
     * 
     * @note Solution is otherwise assumed to already be feasible.
     * 
     * @param vehicleIndex Specific vehicle which has changed
     */
    void updateFeasibility(int vehicleIndex);

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
     * @brief Calculate the cost of the solution, given that ONLY a specific vehicleIndex changed;
     * 
     * @note Solutions are assumed feasible. No feasibility check will be ran.
     * 
     * @param vehicleIndex Specific vehicle which has changed
     */
    void updateCost(int vehicleIndex);

    /**
     * @brief Returns a copy of the current solution.
     * 
     * @return Copy of solution 
     */
    Solution copy();

    /**
     * @brief Construct a new Solution given representation as vector.
     * 
     * @param representation Given solution representation
     * @param problem Problem to solve
     */
    Solution(std::vector<int> representation, Problem* problem);

    /**
     * @brief Invalidates the current caches.
     * Call whenever solution is modified.
     */
    void invalidateCache();

    Problem* problem;
    std::vector<int> representation;

    std::vector<int> seperators;
    std::vector<int> costs;
    
    std::pair<bool, bool> feasibilityCache;
    std::pair<bool, int> costCache;

    private:
    // Only allow static instance creation, unless created from given vector (above)
    Solution(Problem* problem);
};