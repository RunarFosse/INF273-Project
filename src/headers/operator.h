#pragma once

#include <set>
#include <vector>
#include <random>
#include <algorithm>
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
    virtual Solution apply(Solution* solution, std::default_random_engine& rng) = 0;
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
    Solution apply(Solution* solution, std::default_random_engine& rng);

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
    Solution apply(Solution* solution, std::default_random_engine& rng);

    private:
    std::vector<Operator*> operators;
    std::vector<double> weights;
};

class RandomInsert : public Operator {
    public:
    /**
     * @brief Apply random-insert heuristic operator to solution.
     * Random-insert moves a random amount of calls to a random
     * feasible position.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class RandomBestInsert : public Operator {
    public:
    /**
     * @brief Apply best-insert heuristic operator to solution.
     * Best-insert moves a random amount of calls to the current
     * most optimal positions.
     * 
     * @note The order calls are inserted in is randomized.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class GreedyBestInsert : public Operator {
    public:
    /**
     * @brief Apply best-insert heuristic operator to solution.
     * Best-insert moves a random amount of calls to the current
     * most optimal positions.
     * 
     * @note The order calls are inserted in is greedy.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
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
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

/**
 * @brief Performs the random insert operation on several calls on the given solution.
 * 
 * @param callsToInsert Number of calls to insert
 * @param solution The given solution
 * @param rng Random number generator engine
 * @return Neighbour solution 
 */
Solution* performRandomInsert(int callsToInsert, Solution* solution, std::default_random_engine& rng);

/**
 * @brief Performs the best insert operation on several calls on the given solution.
 * 
 * @param callsToInsert Number of calls to insert
 * @param greedy If the order of calls inserted should be greedy or random.
 * @param solution The given solution
 * @param rng Random number generator engine
 * @return Neighbour solution 
 */
Solution* performBestInsert(int callsToInsert, bool greedy, Solution* solution, std::default_random_engine& rng);

std::vector<std::pair<int, CallDetails>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort);