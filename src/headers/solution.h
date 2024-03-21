#pragma once

#include <set>
#include <queue>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>

#include "problem.h"

struct {
    int vehicle;
    std::pair<int, int> indices;
} typedef CallDetails;

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
     * @brief Strictly add call to vehicle.
     * 
     * @note Temporary function which should be used in conjunction with Solution::remove.
     * 
     * @param vehicleIndex Vehicle to add call to
     * @param callIndex Call to add
     * @param indices Indices to place call
     */
    void add(int vehicleIndex, int callIndex, std::pair<int, int> indices);

    /**
     * @brief Strictly remove call from vehicle.
     * 
     * @note Temporary function which will make solution invalid.
     * 
     * @param vehicleIndex Vehicle to remove call from
     * @param callIndex Call to remove
     */
    void remove(int vehicleIndex, int callIndex);

    /**
     * @brief Moves a call from wherever it currently is to index1 and index2.
     * 
     * @param vehicleIndex Vehicle to move call into
     * @param callIndex Call to move
     * @param indices Insertion indices
     */
    void move(int vehicleIndex, int callIndex, std::pair<int, int> indices);

    /**
     * @brief Outsource a given call. Everything handled automatically.
     * 
     * @param callIndex Call to outsource
     * @return Resulting indices where call was inserted in solution representation
     */
    std::pair<int, int> outsource(int callIndex);

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
     * @brief Calculate the cost of the solution, given that ONLY a specific vehicleIndex changed.
     * 
     * @note Solutions are assumed feasible. No feasibility check will be ran.
     * 
     * @param vehicleIndex Specific vehicle which has changed
     * @param callIndex Specific call which was added/removed
     * @param index1 First changed index
     * @param index2 Second changed index
     * @param insertion Decides if change is an insertion or removal
     * @param previous Previous solution prior to modification
     */
    void updateCost(int vehicleIndex, int callIndex, int index1, int index2, bool insertion, Solution* previous);

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
    
    const Problem* problem;
    const int outsourceVehicle;

    std::vector<std::vector<int>> representation;
    std::vector<CallDetails> callDetails;

    std::vector<int> costs;
    
    std::pair<bool, bool> feasibilityCache;
    std::pair<bool, int> costCache;

    private:
    // Only allow static instance creation, unless created from given vector (above)
    Solution(Problem* problem);
};