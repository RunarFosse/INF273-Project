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

std::vector<std::pair<int, CallDetails>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort) {
    // Create a copy of the solution
    // TODO: Remove use of copy, directly use given solution instead
    Solution current = solution->copy();

    // Get the call and its feasible vehicles
    Call& call = current.problem->calls[callIndex-1];
    std::vector<int>& possibleVehicles = call.possibleVehicles;

    // Initialize a vector for storing feasible insertions
    std::vector<std::pair<int, CallDetails>> feasibleInsertions;

    // Check all insertions within every possible vehicle
    for (int vehicleIndex : possibleVehicles) {
        // Store vehicle details
        std::pair<std::vector<int>, std::vector<int>> details = current.getDetails(vehicleIndex);
        std::vector<int>& times = details.first, capacities = details.second;

        // Store previous cost for easy updates
        int previousCost = current.costs[vehicleIndex-1];

        // Loop over every possible insertion point
        for (int pointer1 = 0; pointer1 < current.representation[vehicleIndex-1].size()+1; pointer1++) {
            // Ensure that current start is feasible
            if (times[pointer1] > call.pickupWindow.end) {
                break;
            }
            if (capacities[pointer1] < call.size) {
                continue;
            }

            int pointer2 = pointer1;
            while (++pointer2 < current.representation[vehicleIndex-1].size()+2) {
                // Ensure that current end is feasible
                if (times[pointer2] > call.deliveryWindow.end) {
                    break;
                }

                // Insert the call at the current position
                current.add(vehicleIndex, callIndex, std::make_pair(pointer1, pointer2));

                // Check feasibility
                current.updateFeasibility(vehicleIndex);

                // If it is feasible, store it
                if (current.isFeasible()) {
                    feasibleInsertions.push_back(std::make_pair(current.getCost(), current.callDetails[callIndex-1]));
                }

                // Then remove the call again
                current.remove(callIndex);
            }
        }
    }

    // Aswell as checking outsource
    current.outsource(callIndex);
    feasibleInsertions.push_back(std::make_pair(current.getCost(), current.callDetails[callIndex-1]));

    // After all insertions, sort the vector by cost in ascending order and return
    if (sort) {
        std::sort(feasibleInsertions.begin(), feasibleInsertions.end(), [](const std::pair<int, CallDetails>& a, const std::pair<int, CallDetails>& b) {
            return a.first < b.first;
        });
    }
    return feasibleInsertions;
}