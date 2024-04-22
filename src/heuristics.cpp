#include "heuristics.h"
#include "debug.h"

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

    // Store every removed costly call
    std::vector<int> callIndices;
    callIndices.reserve(callsToRemove);

    // Iteratively sample every call
    for (int i = 0; i < callsToRemove; i++) {
        // First calculate the cost of each call
        std::vector<std::pair<int, int>> costlyCalls;
        costlyCalls.reserve(solution->problem->noCalls);
        for (int callIndex = 1; callIndex <= solution->problem->noCalls; callIndex++) {
            // Skip removed calls
            if (solution->callDetails[callIndex-1].removed) {
                continue;
            }

            int vehicleCall = solution->callDetails[callIndex-1].vehicle;
            current.updateCost(callIndex, false);

            int cost = solution->costs[vehicleCall-1] - current.costs[vehicleCall-1];
            costlyCalls.push_back(std::make_pair(cost, callIndex));
            current.updateCost(callIndex, true);
        }

        // And sort them from most-costly to least-costly
        std::sort(costlyCalls.begin(), costlyCalls.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first > b.first;
        });

        // Then sample the current most costly and remove it
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
    // Initially calculate all feasible insertion positions for all the calls
    std::map<int, std::vector<std::vector<std::pair<int, CallDetails>>>> feasibleInsertions;
    for (int callIndex : callIndices) {
        // Find all feasible insertions for callIndex sorted from best-to-worst
        feasibleInsertions.insert(std::make_pair(callIndex, calculateFeasibleInsertions(callIndex, solution, true)));
    }

    // Move each call into its best possible position
    while (!callIndices.empty()) {
        int bestCost = INT_MAX, bestCall;
        CallDetails bestInsertion;

        for (int callIndex : callIndices) {
            std::vector<std::vector<std::pair<int, CallDetails>>>& feasibleCallInsertions = feasibleInsertions.find(callIndex)->second;

            for (int vehicleIndex = 1; vehicleIndex <= solution->outsourceVehicle; vehicleIndex++) {
                if (feasibleCallInsertions[vehicleIndex-1].empty()) {
                    continue;
                }

                // Check if inserting the current call is better, if so, remember it
                if (feasibleCallInsertions[vehicleIndex-1][0].first < bestCost) {
                    bestCost = feasibleCallInsertions[vehicleIndex-1][0].first;
                    bestCall = callIndex;
                    bestInsertion = feasibleCallInsertions[vehicleIndex-1][0].second;
                }
            }
        }

        // Store change in cost after insertion
        int deltaCost = bestCost - solution->getCost();

        // Move the current best call into its best position and remove it from the set
        solution->add(bestInsertion.vehicle, bestCall, bestInsertion.indices);
        callIndices.erase(bestCall);

        assert(deltaCost >= 0);

        // If the call was inserted into a vehicle (which is not outsource),
        // and there still exist calls to be inserted, update all other's feasible insertion for that vehicle
        if (bestInsertion.vehicle != solution->outsourceVehicle) {
            for (int callIndex : callIndices) {
                if (feasibleInsertions.find(callIndex)->second[bestInsertion.vehicle-1].empty()) {
                    continue;
                }
                feasibleInsertions.find(callIndex)->second[bestInsertion.vehicle-1] = greedyFeasibleInsertions(bestInsertion.vehicle, callIndex, solution, true);
            }
        }

        // And update the cost of the other insertions to correctly include cost of the next call
        for (int callIndex : callIndices) {
            std::vector<std::vector<std::pair<int, CallDetails>>>& feasibleCallInsertions = feasibleInsertions.find(callIndex)->second;

            for (int vehicleIndex = 1; vehicleIndex <= solution->outsourceVehicle; vehicleIndex++) {
                if (vehicleIndex == bestInsertion.vehicle && vehicleIndex != solution->outsourceVehicle) {
                    continue;
                }
                
                for (std::pair<int, CallDetails>& insertion : feasibleCallInsertions[vehicleIndex-1]) {
                    insertion.first += deltaCost;
                }
            }
        }
    }
}

void insertRegret(std::set<int>& callIndices, Solution* solution, int k) {
   // Initially calculate all feasible insertion positions for all the calls
    std::map<int, std::vector<std::vector<std::pair<int, CallDetails>>>> feasibleInsertions;
    for (int callIndex : callIndices) {
        // Find all feasible insertions for callIndex sorted from best-to-worst
        feasibleInsertions.insert(std::make_pair(callIndex, calculateFeasibleInsertions(callIndex, solution, true)));
    }

    // Move each call into its best possible position
    while (!callIndices.empty()) {
        long long highestRegret = -1, bestCall, bestCost = INT_MAX;
        CallDetails bestInsertion;

        for (int callIndex : callIndices) {
            std::vector<std::vector<std::pair<int, CallDetails>>>& feasibleCallInsertions = feasibleInsertions.find(callIndex)->second;

            // First add all to 1-dimensional sorted vector to easily compare insertions over different vehicles
            std::vector<std::pair<int, CallDetails>> feasibleCallInsertionsUnwrapped;
            for (auto && inner : feasibleCallInsertions) {
                feasibleCallInsertionsUnwrapped.insert(feasibleCallInsertionsUnwrapped.end(), inner.begin(), inner.end());
            }
            std::sort(feasibleCallInsertionsUnwrapped.begin(), feasibleCallInsertionsUnwrapped.end(), [](const std::pair<int, CallDetails>& a, const std::pair<int, CallDetails>& b) {
                return a.first < b.first;
            });

            // Then calculate the regret for the current call
            long long regret = feasibleCallInsertionsUnwrapped[std::min(k, (int)feasibleCallInsertionsUnwrapped.size()-1)].first - feasibleCallInsertionsUnwrapped[0].first;
            // Check if inserting the current call is better, if so, remember it
            if (regret > highestRegret || (regret == highestRegret && feasibleCallInsertionsUnwrapped[0].first < bestCost)) {
                highestRegret = regret;
                bestCall = callIndex;
                bestCost = feasibleCallInsertionsUnwrapped[0].first;
                bestInsertion = feasibleCallInsertionsUnwrapped[0].second;
            }
            assert(regret >= 0);
        }


        // Store change in cost after insertion
        int deltaCost = bestCost - solution->getCost();

        // Move the call with the highest regret into its best position and remove it from the set
        solution->add(bestInsertion.vehicle, bestCall, bestInsertion.indices);
        callIndices.erase(bestCall);

        // If the call was inserted into a vehicle (which is not outsource), update all other's feasible insertion for that vehicle
        if (bestInsertion.vehicle != solution->outsourceVehicle) {
            for (int callIndex : callIndices) {
                feasibleInsertions.find(callIndex)->second[bestInsertion.vehicle-1] = greedyFeasibleInsertions(bestInsertion.vehicle, callIndex, solution, true);
            }
        }

        // And update the cost of the other insertions to correctly include cost of the next call
        for (int callIndex : callIndices) {
            std::vector<std::vector<std::pair<int, CallDetails>>>& feasibleCallInsertions = feasibleInsertions.find(callIndex)->second;
            for (int vehicleIndex = 1; vehicleIndex <= solution->outsourceVehicle; vehicleIndex++) {
                if (vehicleIndex == bestInsertion.vehicle && vehicleIndex != solution->outsourceVehicle) {
                    continue;
                }
                
                for (std::pair<int, CallDetails>& insertion : feasibleCallInsertions[vehicleIndex-1]) {
                    insertion.first += deltaCost;
                }
            }
        }
    }
}

void insertBeam(std::set<int>& callIndices, Solution* solution, int width) {
    // Keep track of the beam-width number of best solutions, together with its set of not yet inserted calls
    std::vector<std::pair<Solution, std::pair<std::queue<std::pair<int, CallDetails>>, std::set<int>>>> beam;
    beam.push_back(std::make_pair(solution->copy(), std::make_pair(std::queue<std::pair<int, CallDetails>>(), callIndices)));

    for (int i = 0; i < callIndices.size(); i++) {
        // Store the current most promising solutions
        std::vector<std::pair<int, std::pair<Solution, std::pair<std::queue<std::pair<int, CallDetails>>, std::set<int>>>>> temporary;
        temporary.reserve(beam.size() * width);

        for (auto && [current, info] : beam) {
            std::queue<std::pair<int, CallDetails>>& steps = info.first;
            std::set<int>& calls = info.second;

            // Initially calculate all feasible insertion positions for all the calls
            std::vector<std::pair<int, std::pair<int, CallDetails>>> bestInsertions;
            bestInsertions.reserve(calls.size() * width);

            for (int callIndex : calls) {
                // Find all feasible insertions for callIndex sorted from best-to-worst, unwrap them
                std::vector<std::pair<int, CallDetails>> feasibleCallInsertionsUnwrapped;
                for (auto && inner : calculateFeasibleInsertions(callIndex, &current, true)) {
                    feasibleCallInsertionsUnwrapped.insert(feasibleCallInsertionsUnwrapped.end(), inner.begin(), inner.end());
                }
                std::sort(feasibleCallInsertionsUnwrapped.begin(), feasibleCallInsertionsUnwrapped.end(), [](const std::pair<int, CallDetails>& a, const std::pair<int, CallDetails>& b) {
                    return a.first < b.first;
                });

                // and remember the (up to) beam-width best ones
                for (int j = 0; j < std::min(width, (int)feasibleCallInsertionsUnwrapped.size()); j++) {
                    bestInsertions.push_back(std::make_pair(callIndex, feasibleCallInsertionsUnwrapped[j]));
                }
            }

            // Then sort this to get only the best of the best
            std::sort(bestInsertions.begin(), bestInsertions.end(), [](const std::pair<int, std::pair<int, CallDetails>>& a, const std::pair<int, std::pair<int, CallDetails>>& b) {
                return a.second.first < b.second.first;
            });

            // Store the beam-width best ones in temporary solution storage
            for (int j = 0; j < std::min(width, (int)bestInsertions.size()); j++) {
                int callIndex = bestInsertions[j].first;
                CallDetails insertion = bestInsertions[j].second.second;

                Solution copy = current.copy();
                copy.add(insertion.vehicle, callIndex, insertion.indices);

                std::queue<std::pair<int, CallDetails>> stepsCopy = steps;
                stepsCopy.push(std::make_pair(callIndex, insertion));
                std::set<int> callsCopy(calls.begin(), calls.end());
                callsCopy.erase(callIndex);
                temporary.push_back(std::make_pair(copy.getCost(), std::make_pair(copy, std::make_pair(stepsCopy, callsCopy))));
            }
        }

        // Then sort all temporary solutions by cost and pick the beam-width best ones
        std::sort(temporary.begin(), temporary.end(), [](const std::pair<int, std::pair<Solution, std::pair<std::queue<std::pair<int, CallDetails>>, std::set<int>>>>& a, const std::pair<int, std::pair<Solution, std::pair<std::queue<std::pair<int, CallDetails>>, std::set<int>>>>& b) {
            return a.first < b.first;
        });

        beam.clear();
        for (int j = 0; beam.size() < std::min(width, (int)temporary.size()); j++) {
            beam.push_back(temporary[j].second);
        }
    }

    // Now we have the best solution found through beam search
    std::queue<std::pair<int, CallDetails>> steps = beam[0].second.first;

    // TODO: Insert all the calls into solution given this best solution
    while (!steps.empty()) {
        auto && [callIndex, insertion] = steps.front();
        steps.pop();
        solution->add(insertion.vehicle, callIndex, insertion.indices);
        //Debugger::printToTerminal("Moving " + std::to_string(callIndex) + " to " + std::to_string(insertion.vehicle) + " (" + std::to_string(insertion.indices.first) + ", " + std::to_string(insertion.indices.second) + ")\n");
    }
}

void insertRandom(std::set<int>& callIndices, Solution* solution, std::default_random_engine& rng) {
    // Initially calculate all feasible insertion positions for all the calls
    std::map<int, std::vector<std::vector<std::pair<int, CallDetails>>>> feasibleInsertions;
    for (int callIndex : callIndices) {
        // Find all feasible insertions for callIndex sorted from best-to-worst
        feasibleInsertions.insert(std::make_pair(callIndex, calculateFeasibleInsertions(callIndex, solution, false)));
    }

    // Move each call into its best possible position
    while (!callIndices.empty()) {
        // Select a random call
        std::vector<int> out;
        std::sample(callIndices.begin(), callIndices.end(), std::back_inserter(out), 1, rng);
        int callIndex = out[0];

        std::vector<std::vector<std::pair<int, CallDetails>>>& feasibleCallInsertions = feasibleInsertions.find(callIndex)->second;

            // Add all to 1-dimensional vector to easily sample random insertions over different vehicles
        std::vector<std::pair<int, CallDetails>> feasibleCallInsertionsUnwrapped;
        for (auto && inner : feasibleCallInsertions) {
            feasibleCallInsertionsUnwrapped.insert(feasibleCallInsertionsUnwrapped.end(), inner.begin(), inner.end());
        }

        // And select a random insertion
        CallDetails insertion = feasibleCallInsertionsUnwrapped[std::uniform_int_distribution<int>(0, feasibleCallInsertionsUnwrapped.size()-1)(rng)].second;

        // Move the call with the highest regret into its best position and remove it from the set
        solution->add(insertion.vehicle, callIndex, insertion.indices);
        callIndices.erase(callIndex);

        // If the call was inserted into a vehicle (which is not outsource), update all other's feasible insertion for that vehicle
        if (insertion.vehicle != solution->outsourceVehicle) {
            for (int callIndex : callIndices) {
                feasibleInsertions.find(callIndex)->second[insertion.vehicle-1] = greedyFeasibleInsertions(insertion.vehicle, callIndex, solution, false);
            }
        }
    }
}

std::vector<std::vector<std::pair<int, CallDetails>>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort) {
    // Get the call and its feasible vehicles
    Call& call = solution->problem->calls[callIndex-1];
    std::vector<int>& possibleVehicles = call.possibleVehicles;

    // Initialize a vector for storing feasible insertions for each vehicle
    std::vector<std::vector<std::pair<int, CallDetails>>> feasibleInsertions;
    feasibleInsertions.resize(solution->problem->noVehicles);

    // Check all insertions within every possible vehicle
    for (int vehicleIndex : possibleVehicles) {
        feasibleInsertions[vehicleIndex-1] = greedyFeasibleInsertions(vehicleIndex, callIndex, solution, sort);
    }

    // Aswell as checking outsource
    solution->outsource(callIndex);
    solution->updateFeasibility(solution->outsourceVehicle);
    std::vector<std::pair<int, CallDetails>> outsourceInsertion;
    outsourceInsertion.push_back(std::make_pair(solution->getCost(), solution->callDetails[callIndex-1]));
    feasibleInsertions.push_back(outsourceInsertion);

    // Remove call again before returning
    solution->remove(callIndex);

    return feasibleInsertions;
}

std::vector<std::pair<int, CallDetails>> greedyFeasibleInsertions(int vehicleIndex, int callIndex, Solution* solution, bool sort) {
    // Get the call and its feasible vehicles
    Call& call = solution->problem->calls[callIndex-1];
    std::unordered_set<int>& possibleVehiclesSet = call.possibleVehiclesSet;

    // Initialize a vector for storing feasible insertions for this vehicle
    std::vector<std::pair<int, CallDetails>> feasibleInsertions;

    // If vehicle is not a feasible vehicle, return early
    if (possibleVehiclesSet.find(vehicleIndex) == possibleVehiclesSet.end()) {
        return feasibleInsertions;
    }

    // Store initial vehicle details
    std::pair<std::vector<int>, std::vector<int>> details = solution->getDetails(vehicleIndex, call.pickupWindow.end);
    std::vector<int>& times = details.first, capacities = details.second;

    // Loop over every possible insertion point
    for (int pointer1 = 0; pointer1 < solution->representation[vehicleIndex-1].size()+1; pointer1++) {
        for (int pointer2 = pointer1+1; pointer2 < solution->representation[vehicleIndex-1].size()+2; pointer2++) {

            // Insert the call at the current position
            solution->add(vehicleIndex, callIndex, std::make_pair(pointer1, pointer2));

            // Check feasibility
            std::pair<int, bool> feasibilityInformation = solution->updateFeasibility(vehicleIndex, pointer1, times[pointer1], capacities[pointer1]);

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

    // Update feasibility
    solution->updateFeasibility(vehicleIndex);

    // After all insertions, sort the vector by cost in ascending order and return
    if (sort) {
        std::sort(feasibleInsertions.begin(), feasibleInsertions.end(), [](const std::pair<int, CallDetails>& a, const std::pair<int, CallDetails>& b) {
            return a.first < b.first;
        });
    }
    return feasibleInsertions;
}