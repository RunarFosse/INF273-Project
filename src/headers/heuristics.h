#pragma once

#include "solution.h"

/**
 * @brief Remove similar calls using precalculated relatedness 
 * measures from the current solution.
 * 
 * @param callsToRemove Number of calls to remove
 * @param solution Solution to remove in
 * @param rng Random number generator engine
 * @return Vector of callIndices removed
 */
std::vector<int> removeSimilar(int callsToRemove, Solution* solution, std::default_random_engine& rng);

/**
 * @brief Remove most costly calls from the current solution.
 * 
 * @param callsToRemove Number of calls to remove
 * @param solution Solution to remove in
 * @param rng Random number generator engine
 * @return Vector of callIndices removed
 */
std::vector<int> removeCostly(int callsToRemove, Solution* solution, std::default_random_engine& rng);

/**
 * @brief Remove random calls from the current solution.
 * 
 * @param callsToRemove Number of calls to remove
 * @param solution Solution to remove in
 * @param rng Random number generator engine
 * @return Vector of callIndices removed
 */
std::vector<int> removeRandom(int callsToRemove, Solution* solution, std::default_random_engine& rng);