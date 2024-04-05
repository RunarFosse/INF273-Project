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
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    virtual Solution apply(Solution* solution, int iteration, std::default_random_engine& rng) = 0;
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
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);

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
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);

    private:
    std::vector<Operator*> operators;
    std::vector<double> weights;
};

class AdaptiveOperator : public Operator {
    public:
    /**
     * @brief Create an Adaptive Operator.
     * It adaptively updates the probability for each sub-operator to be called,
     * calculated from the efficacy of each of them.
     * 
     * @param operators Operators to apply, with a calculated adaptive probability
     */
    AdaptiveOperator(std::vector<Operator*> operators);

    /**
     * @brief Apply an operator to solution.
     * Operator is chosen based on adaptive scoring system.
     * 
     * @param solution Solution to apply an operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);

    private:
    std::vector<Operator*> operators;
    std::vector<double> weights;
    std::vector<int> scores;
    std::vector<int> uses;

    // HashSet and HashFunction for solutions
    struct SolutionHash {
        size_t operator()(const Solution& solution) const {
            std::hash<int> hasher;
            size_t seed = 0;
            for (auto&& representation : solution.representation) {
                // Hash each vehicle
                for (int callIndex : representation) {
                    seed ^= hasher(callIndex) + 0x9e3779b9 + (seed<<6) + (seed>>2);
                }
                // Add a seperation
                seed ^= hasher(0) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            }
            return seed;
        }
    };
    std::unordered_set<Solution, SolutionHash> seenSolutions;

    int bestCost = 0;
    double r = 0.2;

    /**
     * @brief Update each operator's weight based on previous performance.
     */
    void update();

    /**
     * @brief Reset each operator's weight.
     */
    void reset();
};

class SimilarGreedyInsert : public Operator {
    public:
    /**
     * @brief Similar-greedy-insert is an operator which
     * selects similar calls, and inserts them in the best
     * possible position given by a greedy order.
     * 
     * @param solution Solution to apply operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);
};

class SimilarRegretInsert : public Operator {
    public:
    /**
     * @brief Similar-regret-insert is an operator which
     * selects similar calls, and inserts them in the best
     * possible position given by a regret-k order.
     * 
     * @param solution Solution to apply operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);
};

class CostlyGreedyInsert : public Operator {
    public:
    /**
     * @brief Similar-greedy-insert is an operator which
     * selects the most costly calls, and inserts them in the best
     * possible position given by a greedy order.
     * 
     * @param solution Solution to apply operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);
};

class CostlyRegretInsert : public Operator {
    public:
    /**
     * @brief Similar-regret-insert is an operator which
     * selects the most costly calls, and inserts them in the best
     * possible position given by a regret-k order.
     * 
     * @param solution Solution to apply operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);
};

class RandomGreedyInsert : public Operator {
    public:
    /**
     * @brief Similar-greedy-insert is an operator which
     * randomly selects several calls, and inserts them in the best
     * possible position given by a greedy order.
     * 
     * @param solution Solution to apply operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);
};

class RandomRegretInsert : public Operator {
    public:
    /**
     * @brief Similar-regret-insert is an operator which
     * randomly selects several calls, and inserts them in the best
     * possible position given by a regret-k order.
     * 
     * @param solution Solution to apply operator on
     * @param iteration Current iteration
     * @param rng Random number generator Engine
     * @return Neighbour solution
     */
    Solution apply(Solution* solution, int iteration, std::default_random_engine& rng);
};

/**
 * @brief Sample an integer from a uniform distribution,
 * optimally calculated from solution's problem.
 * 
 * @param solution Given solution
 * @param iteration Current iteration
 * @param rng Random number generator engine
 * @return Sampled integer 
 */
int boundedUniformSample(Solution* solution, int iteration, std::default_random_engine& rng);