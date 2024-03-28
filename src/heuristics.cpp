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