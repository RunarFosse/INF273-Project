#include "heuristics.h"

std::vector<int> removeSimilar(int callsToRemove, Solution* solution, std::default_random_engine& rng) {
    // Initialize a vector to hold all removed calls
    std::vector<int> callIndices;
    callIndices.reserve(callsToRemove);

    // Initially remove a random call
    int initialCallIndex = std::uniform_int_distribution<int>(1, solution->problem->noCalls)(rng);
    solution->remove(initialCallIndex);
    callIndices.push_back(initialCallIndex);

    // Remove the remaining next-most similar calls
    for (int i = 0; i < callsToRemove-1; i++) {
        int callIndex = solution->problem->calls[initialCallIndex-1].similarities[i].callIndex;
        solution->remove(callIndex);
        callIndices.push_back(callIndex);
    }

    // Return all removed calls
    return callIndices;
}

std::vector<int> removeCostly(int callsToRemove, Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // First calculate the cost of each call
    std::vector<std::pair<int, int>> costlyCalls;
    costlyCalls.reserve(solution->problem->noCalls);
    for (int callIndex = 1; callIndex <= solution->problem->noCalls; callIndex++) {
        int vehicleCall = solution->callDetails[callIndex-1].vehicle;
        current.updateCost(callIndex, false);

        int cost = solution->costs[vehicleCall-1] - current.costs[vehicleCall-1];
        costlyCalls.push_back(std::make_pair(cost, callIndex));
    }

    // And sort them from most-costly to least-costly
    std::sort(costlyCalls.begin(), costlyCalls.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.first > b.first;
    });

    // Then sample the most costly, remove them and return
    std::vector<int> callIndices;
    callIndices.reserve(callsToRemove);
    for (int i = 0; i < callsToRemove; i++) {
        int callIndex = costlyCalls[i].second;
        solution->remove(callIndex);
        callIndices.push_back(callIndex);
    }

    // Return all removed calls
    return callIndices;
}

std::vector<int> removeRandom(int callsToRemove, Solution* solution, std::default_random_engine& rng) {
    // Initialize a vector to hold all removed calls
    std::vector<int> callIndices;
    callIndices.reserve(callsToRemove);

    // Randomly sample all calls to remove 
    // "https://stackoverflow.com/a/3724708"
    std::unordered_set<int> sampledCalls;
    int total = solution->problem->noCalls+1;
    for (int i = total - callsToRemove; i < total; i++) {
        int callIndex = std::uniform_int_distribution<int>(1, i)(rng); 
        if (sampledCalls.find(callIndex) != sampledCalls.end()) {
            callIndex = i;
        }
        sampledCalls.insert(callIndex);

        // Remove the call and add them to vector
        solution->remove(callIndex);
        callIndices.push_back(callIndex);
    }

    // Return all removed calls
    return callIndices;
}

void insertGreedy(std::set<int>& callIndices, Solution* solution) {
    // Move each call into its best possible position
    while (!callIndices.empty()) {
        int bestCost = INT_MAX, bestCall;
        CallDetails bestInsertion;

        for (int callIndex : callIndices) {
            // Find all feasible insertions sorted from best-to-worst
            std::vector<std::pair<int, CallDetails>> feasibleInsertions = calculateFeasibleInsertions(callIndex, solution, true);

            // Check if inserting the current call is better, if so, remember it
            if (feasibleInsertions[0].first < bestCost) {
                bestCost = feasibleInsertions[0].first;
                bestCall = callIndex;
                bestInsertion = feasibleInsertions[0].second;
            }
        }

        // Move the current best call into its best position and remove it from the set
        solution->add(bestInsertion.vehicle, bestCall, bestInsertion.indices);
        callIndices.erase(bestCall);
    }
}

void insertRegret(std::set<int>& callIndices, Solution* solution, int k) {
    // Move each call into its best possible position
    while (!callIndices.empty()) {
        int highestRegret = -1, bestCall;
        CallDetails bestInsertion;

        for (int callIndex : callIndices) {
            // Find all feasible insertions sorted from best-to-worst
            std::vector<std::pair<int, CallDetails>> feasibleInsertions = calculateFeasibleInsertions(callIndex, solution, true);

            // Calculate the regret for the current call
            int regret = 0;
            for (int i = 0; i < k; i++) {
                if (i+1 >= feasibleInsertions.size()) {
                    break;
                }
                regret += feasibleInsertions[i+1].first - feasibleInsertions[i].first;
            }

            // Check if inserting the current call is better, if so, remember it
            if (highestRegret < regret) {
                highestRegret = regret;
                bestCall = callIndex;
                bestInsertion = feasibleInsertions[0].second;
            }
        }

        // Move the call with the highest regret into its best position and remove it from the set
        solution->add(bestInsertion.vehicle, bestCall, bestInsertion.indices);
        callIndices.erase(bestCall);
    }
}

std::vector<std::pair<int, CallDetails>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort) {
    // Get the call and its feasible vehicles
    Call& call = solution->problem->calls[callIndex-1];
    std::vector<int>& possibleVehicles = call.possibleVehicles;

    // Initialize a vector for storing feasible insertions
    std::vector<std::pair<int, CallDetails>> feasibleInsertions;

    // Check all insertions within every possible vehicle
    for (int vehicleIndex : possibleVehicles) {

        // Loop over every possible insertion point
        for (int pointer1 = 0; pointer1 < solution->representation[vehicleIndex-1].size()+1; pointer1++) {
            for (int pointer2 = pointer1+1; pointer2 < solution->representation[vehicleIndex-1].size()+2; pointer2++) {

                // Insert the call at the current position
                solution->add(vehicleIndex, callIndex, std::make_pair(pointer1, pointer2));

                // Check feasibility
                std::pair<int, bool> feasibilityInformation = solution->updateFeasibility(vehicleIndex);

                // If it is feasible, store it
                if (solution->isFeasible()) {
                    feasibleInsertions.push_back(std::make_pair(solution->getCost(), solution->callDetails[callIndex-1]));

                // If not, use infeasibility information to decide how we should check further positions
                } else {

                    // If due to capacity, check next pickup index
                    if (feasibilityInformation.second) {
                        pointer2 = solution->representation[vehicleIndex-1].size()+2;
                    
                    // Else if due to time window exceeded
                    } else {
                        // If at pickup index, break (pickup is outside pickup window)
                        if (feasibilityInformation.first == pointer1) {
                            pointer1 = solution->representation[vehicleIndex-1].size()+1;
                            pointer2 = solution->representation[vehicleIndex-1].size()+2;
                        
                        // Else if at or before delivery index, break inner (delivery is outside delivery window or pickup makes another pickup/delivery infeasible)
                        } else if (feasibilityInformation.first <= pointer2) {
                            pointer2 = solution->representation[vehicleIndex-1].size()+2;
                        } 
                    }
                }

                // Then remove the call again
                solution->remove(callIndex);
            }
        }
    }

    // Aswell as checking outsource
    solution->outsource(callIndex);
    solution->updateFeasibility(solution->outsourceVehicle);
    feasibleInsertions.push_back(std::make_pair(solution->getCost(), solution->callDetails[callIndex-1]));

    // Remove call again before returning
    solution->remove(callIndex);

    // After all insertions, sort the vector by cost in ascending order and return
    if (sort) {
        std::sort(feasibleInsertions.begin(), feasibleInsertions.end(), [](const std::pair<int, CallDetails>& a, const std::pair<int, CallDetails>& b) {
            return a.first < b.first;
        });
    }
    return feasibleInsertions;
}