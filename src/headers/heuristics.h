#pragma once

#include <map>

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
 * @param callIndices Set of calls to insert
 * @param solution Solution to insert into
 */
void insertGreedy(std::set<int>& callIndices, Solution* solution);

/**
 * @brief Insert all given calls into their best possible positions.
 * Calls are inserted in a regret-k order.
 * 
 * @param callIndices Set of calls to insert
 * @param solution Solution to insert into
 * @param k Regret-k parameter
 */
void insertRegret(std::set<int>& callIndices, Solution* solution, int k);

/**
 * @brief Insert all given calls into a random, feasible position.
 * 
 * @param callIndices Set of calls to insert
 * @param solution Solution to insert into
 */
void insertRandom(std::set<int>& callIndices, Solution* solution, std::default_random_engine& rng);

/**
 * @brief Calculate all different insertion positions for a given call.
 * 
 * @param callIndex Given call to find insertions for
 * @param solution Solution to find insertion positions in
 * @param sort Sort the positions by cost in an ascending order 
 * @return Vector of vectors of (cost, callDetail) for each insertion position for each vehicle
 */
std::vector<std::vector<std::pair<int, CallDetails>>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort);

/**
 * @brief Calculate all different insertion positions for a given call, inside a single vehicle.
 * 
 * @param vehicleIndex Given vehicle to find insertion in
 * @param callIndex Given call to find insertions for
 * @param solution Solution to find insertion positions in
 * @param sort Sort the positions by cost in an ascending order 
 * @return Vector of (cost, callDetail) for each insertion position in the vehicle
 */
std::vector<std::pair<int, CallDetails>> greedyFeasibleInsertions(int vehicleIndex, int callIndex, Solution* solution, bool sort);