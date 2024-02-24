#include "operator.h"

#include "debug.h"

Solution OneInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    solution = solution.copy();

    // Select a random call
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noCalls)(rng);

    // Find vehicle which currently has callIndex
    int currentVehicle = 1, callVehicle;
    for (int i = 0; i < solution.representation.size(); i++) {
        if (solution.representation[i] == callIndex) {
            callVehicle = currentVehicle;
        } else if (solution.representation[i] == 0) {
            currentVehicle++;
        }
    }

    // Retrieve the list of possible vehicles for this call
    std::vector<int> possibleVehicles;
    for (int vehicleIndex : solution.problem->calls[callIndex-1].possibleVehicles) {
        if (vehicleIndex == callVehicle) {
            continue;
        }
        possibleVehicles.push_back(vehicleIndex);
    }


    // Decide if we should outsource or give to our vehicles
    int vehicleIndex, insert1, insert2;
    double outsourceProbability = 1.0 / (1.2 * (solution.problem->noVehicles+1));
    if (possibleVehicles.empty() || (callVehicle != solution.problem->noVehicles+1 && std::uniform_real_distribution<double>(0, 1)(rng) < outsourceProbability)) {
        // Outsource callIndex
        if (vehicleIndex == callIndex) {
            // Early stop if already at outsource
            return solution;
        }
        vehicleIndex = solution.problem->noVehicles+1;
        for (int i = solution.representation.size()-1; i >= 0; i--) {
            if (solution.representation[i] <= callIndex) {
                insert2 = i;
                insert1 = i-1;
                break;
            }
        }
    } else {
        // Then select a different random vehicle to take this call instead
        vehicleIndex = possibleVehicles[std::uniform_int_distribution<std::size_t>(0, possibleVehicles.size()-1)(rng)];
        int startIndex = solution.seperators[vehicleIndex-1]+1, endIndex = solution.seperators[vehicleIndex]-1;

        // Modify index range based on current position of call
        if (callVehicle < vehicleIndex) {
            startIndex -= 2;
        } else {
            endIndex += 2;
        }

        std::uniform_int_distribution<std::size_t> distribution(startIndex, endIndex);
        insert1 = distribution(rng), insert2 = distribution(rng);
        if (insert1 == insert2) {
            if (insert1 == endIndex) {
                insert1--;
            } else {
                insert2++;
            }
        }
    }

    // Insert the callIndex under the new vehicleIndex at random positions
    solution.move(callIndex, insert1, insert2);

    // Invalidate the caches as we've modified the representation
    solution.invalidateCache();

    // Then greedily update the feasibility for modified vehicles
    solution.updateFeasibility(callVehicle);
    solution.updateFeasibility(vehicleIndex);

    // Also greedily update the cost for the same vehicles
    solution.updateCost(callVehicle);
    solution.updateCost(vehicleIndex);

    // Return the modified neighbour solution
    return solution;
}

Solution BestInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Store best values
    int bestCost = INT_MAX;
    int bestCall, bestVehicle, bestVehicleCall;
    std::pair<int, int> bestIndices;

    // Check every call
    for (int callIndex = 1; callIndex <= solution.problem->noCalls; callIndex++) {
        // Create a copy of the current solution for moving current call
        Solution current = solution.copy();

        // Find the current vehicle which has this call
        int vehicleCall = current.getVehicleWith(callIndex);

        // Check to place within any vehicle (not outsource)
        for (int vehicleIndex = 1; vehicleIndex <= solution.problem->noVehicles; vehicleIndex++) {
            // Verify that vehicle can actually take call
            if (std::find(solution.problem->calls[callIndex-1].possibleVehicles.begin(), solution.problem->calls[callIndex-1].possibleVehicles.end(), vehicleIndex) == solution.problem->calls[callIndex-1].possibleVehicles.end()) {
                continue;
            }

            // Move call to start of current vehicle as default before starting
            int temporaryStart = current.seperators[vehicleIndex-1];
            current.move(callIndex, temporaryStart+1, temporaryStart+2);

            int startIndex = current.seperators[vehicleIndex-1]+1, endIndex = current.seperators[vehicleIndex];
            int pointer1 = startIndex;
            while (pointer1 < endIndex-1) {
                int pointer2 = pointer1;
                while (++pointer2 < endIndex) {
                    // Move call
                    current.greedyMove(callIndex, startIndex, endIndex, pointer1, pointer2);

                    // Check feasibility
                    current.updateFeasibility(vehicleIndex);
                    if (vehicleIndex != vehicleCall) {
                        current.updateFeasibility(vehicleCall);
                    }
                    if (!current.isFeasible()) {
                        continue;
                    }

                    // If feasible, update cost and store if best (and if different)
                    current.updateCost(vehicleIndex);
                    if (vehicleIndex != vehicleCall) {
                        current.updateCost(vehicleCall);
                    }
                    if (current.getCost() < bestCost && current.representation != solution.representation) {
                        bestCost = current.getCost();
                        bestCall = callIndex;
                        bestVehicle = vehicleIndex;
                        bestVehicleCall = vehicleCall;
                        bestIndices = std::make_pair(pointer1, pointer2);
                    }
                }
                pointer1++;
            }
        }
    }

    // If nothing has been found, outsource the best one
    if (bestCost == INT_MAX) {
        for (int callIndex = 1; callIndex <= solution.problem->noCalls; callIndex++) {
            Solution current = solution.copy();
            std::pair<int, int> indices = current.outsource(callIndex);
            int vehicleCall = current.getVehicleWith(callIndex);
            if (current.getCost() < bestCost && current.representation != solution.representation) {
                bestCost = current.getCost();
                bestCall = callIndex;
                bestVehicle = solution.problem->noVehicles+1;
                bestVehicleCall = vehicleCall;
                bestIndices = indices;
            }
        }
    }

    // At end, move bestCall to bestIndices and return new solution
    solution = solution.copy();
    solution.move(bestCall, bestIndices.first, bestIndices.second);
    solution.updateFeasibility(bestVehicle);
    solution.updateCost(bestVehicle);
    if (bestVehicle != bestVehicleCall) {
        solution.updateFeasibility(bestVehicleCall);
        solution.updateCost(bestVehicleCall);
    }

    return solution;
}