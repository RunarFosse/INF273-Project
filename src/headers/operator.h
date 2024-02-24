#pragma once

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