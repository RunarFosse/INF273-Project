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

/**
 * @brief Insert all given calls into their best possible positions.
 * Calls are inserted in a greedy-order.
 * 
 * @param callIndices Calls to insert
 * @param solution Solution to insert into
 */
void insertGreedy(std::vector<int> callIndices, Solution* solution);

/**
 * @brief Insert all given calls into their best possible positions.
 * Calls are inserted in a regret-k order.
 * 
 * @param callIndices Calls to insert
 * @param solution Solution to insert into
 * @param k Regret-k parameter
 */
void insertRegret(std::vector<int> callIndices, Solution* solution, int k);

/**
 * @brief Calculate all different insertion positions for a given call.
 * 
 * @param callIndex Given call to find insertions for
 * @param solution Solution to find insertion positions in
 * @param sort Sort the positions by cost in an ascending order 
 * @return Vector of (cost, callDetail) for each insertion position
 */
std::vector<std::pair<int, CallDetails>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort);