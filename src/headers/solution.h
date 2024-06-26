#pragma once

#include <set>
#include <queue>
#include <memory>
#include <vector>
#include <random>
#include <cassert>
#include <algorithm>
#include <functional>
#include <unordered_set>

#include "problem.h"

struct {
    int vehicle;
    std::pair<int, int> indices;
    bool removed;
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
     * @brief Strictly remove call from representation.
     * 
     * @note Temporary function which will make solution invalid.
     * 
     * @param callIndex Call to remove
     */
    void remove(int callIndex);

    /**
     * @brief Moves a call from wherever it currently is to index1 and index2.
     * 
     * @note Updates cost for you!
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
     * @param startIndex Greedy start of feasibility calculation
     * @param startTime Current time at greedy start of feasibility calculation
     * @param startCapacity Current capacity at greedy start of feasibility calculation
     * @return Pair (index of failure, true if due to capacity else false)
     */
    std::pair<int, bool> updateFeasibility(int vehicleIndex, int startIndex = 0, int startTime = -1, int startCapacity = -1);

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
     * @param callIndex Specific call which was added/removed
     * @param insertion Decides if change is an insertion or removal
     */
    void updateCost(int callIndex, bool insertion);

    /**
     * @brief Get the times and capacities of a given vehicle.
     * 
     * @param vehicleIndex Given vehicle
     * @param timeConstraint Stop when this time is reached
     * @return Returns the times and capacities as a pair (times, capacities)
     */
    std::pair<std::vector<int>, std::vector<int>> getDetails(int vehicleIndex, int timeConstraint);

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
    
    int outsourceVehicle;

    Problem* problem;
    std::vector<std::vector<int>> representation;
    std::vector<CallDetails> callDetails;

    std::vector<int> costs;
    
    std::pair<bool, bool> feasibilityCache;
    std::pair<bool, int> costCache;

    // Change equality operatior
    bool operator==(const Solution& other) const {
        return this->representation == other.representation;
    };

    private:
    // Only allow static instance creation, unless created from given vector (above)
    Solution(Problem* problem);
};