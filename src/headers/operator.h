#pragma once

#include <set>
#include <vector>
#include <random>

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

class BestInsert : public Operator {
    public:
    /**
     * @brief Apply best-insert heuristic operator to solution.
     * Best-insert moves a call to a position such that the new
     * solution is the best possible one could get by doing such a move.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class OneOutsource : public Operator {
    public:
    /**
     * @brief Apply 1-outsource heuristic operator to solution.
     * 1-outsource selects a random, not already outsourced call,
     * and outsources it.
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

class FullOutsource : public Operator {
    public:
    /**
     * @brief Apply full-outsource heuristic operator to solution.
     * full-outsource selects a random vehicle and outsources
     * all its calls.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};

class FullShuffle : public Operator {
    public:
    /**
     * @brief Apply full-shuffle heuristic operator to solution.
     * Full-shuffle selects a random vehicle and shuffles the
     * solution representation part representing it.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution solution, std::default_random_engine& rng);
};