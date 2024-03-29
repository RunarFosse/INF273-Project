#pragma once

#include "problem.h"
#include "solution.h"
#include "heuristics.h"

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

class SimilarGreedyInsert : public Operator {
    public:
    /**
     * @brief Similar-greedy-insert is an operator which
     * selects similar calls, and inserts them in the best
     * possible position given by a greedy order.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class SimilarRegretInsert : public Operator {
    public:
    /**
     * @brief Similar-regret-insert is an operator which
     * selects similar calls, and inserts them in the best
     * possible position given by a regret-k order.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class CostlyGreedyInsert : public Operator {
    public:
    /**
     * @brief Similar-greedy-insert is an operator which
     * selects the most costly calls, and inserts them in the best
     * possible position given by a greedy order.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class CostlyRegretInsert : public Operator {
    public:
    /**
     * @brief Similar-regret-insert is an operator which
     * selects the most costly calls, and inserts them in the best
     * possible position given by a regret-k order.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class RandomGreedyInsert : public Operator {
    public:
    /**
     * @brief Similar-greedy-insert is an operator which
     * randomly selects several calls, and inserts them in the best
     * possible position given by a greedy order.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};

class RandomRegretInsert : public Operator {
    public:
    /**
     * @brief Similar-regret-insert is an operator which
     * randomly selects several calls, and inserts them in the best
     * possible position given by a regret-k order.
     * 
     * @param solution Solution to apply operator on
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, std::default_random_engine& rng);
};


/**
 * @brief Sample an integer from a normal distribution,
 * clamped to the given solution's problem.
 * 
 * @param mean Mean normal distribution parameter
 * @param std Standard deviation normal distribution parameter
 * @param solution Given solution
 * @param rng Random number generator engine
 * @return int 
 */
int normalSample(double mean, double std, Solution* solution, std::default_random_engine& rng);