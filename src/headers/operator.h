#pragma once

#include <set>
#include <vector>
#include <random>
#include <unordered_map>

#include "solution.h"
#include "problem.h"

class Operator {
    public:
    /**
     * @brief Apply operator to solution.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    virtual Solution apply(Solution solution, std::default_random_engine& rng) = 0;
};

class UniformOperator : public Operator {
    public:
    /**
     * @brief Create a Uniform Operator.
     * Applying this has a uniform probability of applying any operator it contains.
     * 
     * @param operators Operators to apply, with a uniform probability each
     */
    UniformOperator(std::vector<Operator*> operators);

    /**
     * @brief Apply an operator to solution.
     * Operator is chosen from any it contains with uniform probability.
     * 
     * @param solution Solution to apply an operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);

    private:
    std::vector<Operator*> operators;
};

class WeightedOperator : public Operator {
    public:
    /**
     * @brief Create a Weighted Operator.
     * Applying samples a weighted probability distribution and applies the selected operator it contains.
     * 
     * @param operators Operators to apply, with a the given probability weight
     */
    WeightedOperator(std::vector<std::pair<Operator*, double>> operators);

    /**
     * @brief Apply an operator to solution.
     * Operator is chosen from any it contains with weighted probability.
     * 
     * @param solution Solution to apply an operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);

    private:
    std::vector<Operator*> operators;
    std::vector<double> weights;
};

class OneInsert : public Operator {
    public:
    /**
     * @brief Apply 1-insert heuristic operator to solution.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class GreedyInsert : public Operator {
    public:
    /**
     * @brief Apply greedy-insert heuristic operator to solution.
     * Greedy-insert moves a random call to a random vehicle,
     * and places it in the best possible position.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class ConstantBestInsert : public Operator {
    public:
    /**
     * @brief Apply constant-best-insert heuristic operator to solution.
     * Constant-best-insert moves between 1 and 10 calls to the current most optimal positions.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class LowBestInsert : public Operator {
    public:
    /**
     * @brief Apply low-best-insert heuristic operator to solution.
     * Low-best-insert moves a lower amount of calls to the current most optimal positions.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class HighBestInsert : public Operator {
    public:
    /**
     * @brief Apply high-best-insert heuristic operator to solution.
     * High-best-insert moves a higher amount of calls to the current most optimal positions.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class MultiOutsource : public Operator {
    public:
    /**
     * @brief Apply multi-outsource heuristic operator to solution.
     * Multi-outsource selects a random number of non-outsourced calls
     * and outsources them.
     * 
     * @note If all calls are outsourced it returns without modification.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class GreedyOutsource : public Operator {
    public:
    /**
     * @brief Apply greedy-outsource heuristic operator to solution.
     * greedy-outsource selects the best, not already outsourced call,
     * to outsource and outsources it.
     * 
     * @note If all calls are outsourced it returns without modification.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

/**
 * @brief Get all feasible insertion points for a given call and vehicle
 * 
 * @note Does not support outsourcing.
 * 
 * @param callIndex Call to insert
 * @param vehicleIndex Vehicle to insert into
 * @param solution Given solution to modify
 * @return Vector of indices (index1, index2)
 */
std::vector<std::pair<int, int>> getFeasibleInsertions(int callIndex, int vehicleIndex, Solution* solution);

/**
 * @brief Calculate the two best insertion indices together with cost of a given call and vehicle.
 * 
 * @note Does not support outsourcing.
 * 
 * @param callIndex Call to insert
 * @param vehicleIndex Vehicle to insert into
 * @param solution Given solution to modify
 * @return (bestCost, (index1, index2))
 */
std::pair<int, std::pair<int, int>> getBestInsertion(int callIndex, int vehicleIndex, int vehicleCall, Solution* solution, std::default_random_engine& rng);

/**
 * @brief Performs the best insert operation on several calls on the given solution.
 * 
 * @param callsToInsert Number of calls to insert
 * @param solution The given solution
 * @param rng Random number generator engine
 * @return Neighbour solution 
 */
Solution* performBestInsert(int callsToInsert, Solution* solution, std::default_random_engine& rng);