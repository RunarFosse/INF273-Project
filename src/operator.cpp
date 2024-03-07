#include "operator.h"

#include "debug.h"

UniformOperator::UniformOperator(std::vector<Operator*> operators) {
    this->operators = operators;
};

Solution UniformOperator::apply(Solution solution, std::default_random_engine& rng) {
    // Get a random operator from those this contains
    int operatorIndex = std::uniform_int_distribution<std::size_t>(0, this->operators.size()-1)(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, rng);
}

Solution OneInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution.copy();

    // Select a random call
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noCalls)(rng);
    int vehicleIndex = solution.problem->calls[callIndex-1].possibleVehicles[std::uniform_int_distribution<std::size_t>(0, solution.problem->calls[callIndex-1].possibleVehicles.size()-1)(rng)];

    // Find the vehicle which currently has the call
    int vehicleCall = current.getVehicleWith(callIndex);

    // Get all feasible insertion indices
    std::vector<std::pair<int, int>> feasibleIndices = getFeasibleInsertions(callIndex, vehicleIndex, &current);

    // If no feasible insertions exist, return unmodified solution
    if (feasibleIndices.empty()) {
        return solution;
    }

    // Then randomly sample two feasible indices
    std::pair<int, int> index = feasibleIndices[std::uniform_int_distribution<std::size_t>(0, feasibleIndices.size()-1)(rng)];

    // Insert the callIndex under the new vehicleIndex at random positions
    solution.move(callIndex, index.first, index.second);

    // Invalidate the caches as we've modified the representation
    solution.invalidateCache();

    // Then greedily update the feasibility for modified vehicles
    solution.updateFeasibility(vehicleIndex);

    // Also greedily update the cost for the same vehicles
    solution.updateCost(vehicleCall);
    solution.updateCost(vehicleIndex);

    // Return the modified neighbour solution
    return solution;
}

Solution GreedyInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Create a current copy of the solution
    Solution current = solution.copy();

    // Select a random call and feasible vehicle
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noCalls)(rng);
    int vehicleIndex = solution.problem->calls[callIndex-1].possibleVehicles[std::uniform_int_distribution<std::size_t>(0, solution.problem->calls[callIndex-1].possibleVehicles.size()-1)(rng)];

    // Get the best values
    std::pair<int, std::pair<int, int>> bestInsertion = getBestInsertion(callIndex, vehicleIndex, &solution);
    int bestCost = bestInsertion.first;
    std::pair<int, int> bestIndices = bestInsertion.second;

    // Find the vehicle which currently has the call
    int vehicleCall = current.getVehicleWith(callIndex);

    // If no feasible solution has been found, return the solution unmodified
    if (bestCost == INT_MAX) {
        return solution;
    }

    // At end, move callIndex to bestIndices and return new solution
    solution = solution.copy();
    solution.move(callIndex, bestIndices.first, bestIndices.second);
    solution.updateCost(vehicleIndex);
    if (vehicleIndex != vehicleCall) {
        solution.updateCost(vehicleCall);
    }

    solution.invalidateCache();

    return solution;
}

Solution BestInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Store best values
    int bestCost = INT_MAX;
    int bestCall, bestVehicle, bestVehicleCall;
    std::pair<int, int> bestIndices;

    // Check every call
    for (int callIndex = 1; callIndex <= solution.problem->noCalls; callIndex++) {
        // Random dropout, 30%
        if (std::uniform_real_distribution<double>(0, 1)(rng) > 0.3) {
            continue;
        }

        // Create a copy of the current solution for moving current call
        Solution current = solution.copy();

        // Find the current vehicle which has this call
        int vehicleCall = current.getVehicleWith(callIndex);

        // Check to place within any vehicle (not outsource)
        for (int vehicleIndex = 1; vehicleIndex <= solution.problem->noVehicles; vehicleIndex++) {
            // Get the best place to insert
            std::pair<int, std::pair<int, int>> bestInsertion = getBestInsertion(callIndex, vehicleIndex, &solution);

            if (bestInsertion.first < bestCost) {
                bestCost = bestInsertion.first;
                bestCall = callIndex;
                bestVehicle = vehicleIndex;
                bestVehicleCall = vehicleCall;
                bestIndices = bestInsertion.second;
            }
        }
    }

    // If nothing has been found, return solution unmodified
    if (bestCost == INT_MAX) {
        return solution;
    }

    // At end, move bestCall to bestIndices and return new solution
    solution.move(bestCall, bestIndices.first, bestIndices.second);
    solution.updateCost(bestVehicle);
    if (bestVehicle != bestVehicleCall) {
        solution.updateCost(bestVehicleCall);
    }

    return solution;
}

Solution OneOutsource::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    solution = solution.copy();

    // Extract all currently outsourced calls
    std::set<int> outsourcedCalls;
    for (int i = solution.representation.size()-1; i >= 0; i--) {
        if (solution.representation[i] == 0) {
            break;
        }
        outsourcedCalls.insert(solution.representation[i]);
    }

    // Compute all not-outsourced
    std::vector<int> possibleCalls;
    for (int callIndex = 1; callIndex <= solution.problem->noCalls; callIndex++) {
        if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
            possibleCalls.push_back(callIndex);
        }
    }

    // If no call can be outsourced, return current solution
    if (possibleCalls.empty()) {
        return solution;
    }

    // Pick a random possible call to outsource
    int callIndex = possibleCalls[std::uniform_int_distribution<std::size_t>(0, possibleCalls.size()-1)(rng)];
    int vehicleCall = solution.getVehicleWith(callIndex);
    solution.outsource(callIndex);

    // Update costs (as new solution is guaranteed to be feasible)
    solution.updateCost(vehicleCall);
    solution.updateCost(solution.problem->noVehicles+1);

    // Return neighbour solution
    return solution;
}

Solution GreedyOutsource::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    solution = solution.copy();

    // Extract all currently outsourced calls
    std::set<int> outsourcedCalls;
    for (int i = solution.representation.size()-1; i >= 0; i--) {
        if (solution.representation[i] == 0) {
            break;
        }
        outsourcedCalls.insert(solution.representation[i]);
    }

    // Compute all not-outsourced
    std::vector<int> possibleCalls;
    for (int callIndex = 1; callIndex <= solution.problem->noCalls; callIndex++) {
        if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
            possibleCalls.push_back(callIndex);
        }
    }

    // If no call can be outsourced, return current solution
    if (possibleCalls.empty()) {
        return solution;
    }

    // For each call to outsource, find the best
    int bestCost, bestCall, bestVehicleCall;
    for (int callIndex : possibleCalls) {
        Solution current = solution.copy();

        int vehicleCall = current.getVehicleWith(callIndex);

        current.outsource(callIndex);
        current.updateCost(vehicleCall);
        current.updateCost(current.problem->noVehicles+1);

        if (current.getCost() < bestCost) {
            bestCost = current.getCost();
            bestCall = callIndex;
            bestVehicleCall = vehicleCall;
        }
    }

    // Outsource it and return new solution
    solution.outsource(bestCall);
    solution.updateCost(bestVehicleCall);
    solution.updateCost(solution.problem->noVehicles+1);
    return solution;
}

Solution FullOutsource::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    solution = solution.copy();

    // Select a random vehicle
    int vehicleIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noVehicles)(rng);

    // Iterate and outsource all its calls
    int startIndex = solution.seperators[vehicleIndex-1]+1, endIndex = solution.seperators[vehicleIndex];
    for (int i = startIndex; i < endIndex; i += 2) {
        solution.outsource(solution.representation[startIndex]);
    }

    // Invalidate cache and update cost
    solution.invalidateCache();
    solution.updateCost(vehicleIndex);
    solution.updateCost(solution.problem->noVehicles+1);

    return solution;
}

Solution FullShuffle::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    solution = solution.copy();

    // Select a random vehicle
    int vehicleIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noVehicles)(rng);

    // Shuffle its solution representation part
    int startIndex = solution.seperators[vehicleIndex-1]+1, endIndex = solution.seperators[vehicleIndex];
    std::shuffle(solution.representation.begin() + startIndex, solution.representation.begin() + endIndex, rng);

    // Clear the caches and update the cost for the shuffled vehicle
    solution.invalidateCache();
    solution.updateFeasibility(vehicleIndex);
    solution.updateCost(vehicleIndex);

    return solution;
}

std::vector<std::pair<int, int>> getFeasibleInsertions(int callIndex, int vehicleIndex, Solution* solution) {
    // Find thecall and  vehicle which currently has the call
    Call call = solution->problem->calls[callIndex-1];
    int vehicleCall = solution->getVehicleWith(callIndex);

    // Calculate times and capacities of current vehicle
    std::pair<std::vector<int>, std::vector<int>> details = solution->getDetails(vehicleIndex);
    std::vector<int> times = details.first, capacities = details.second;

    // If vehicleCall is not equal to vehicleIndex, extend possible insertions
    int startIndex = solution->seperators[vehicleIndex-1]+1, endIndex = solution->seperators[vehicleIndex];
    if (vehicleCall != vehicleIndex) {
        if (vehicleCall < vehicleIndex) {
            startIndex -= 2;
        } else {
            endIndex += 2;
        }
        times.push_back(times[times.size()-1]);
        times.push_back(times[times.size()-1]);
        capacities.push_back(capacities[capacities.size()-1]);
        capacities.push_back(capacities[capacities.size()-1]);
    }

    // Move call to start of current vehicle as default before starting
    int temporaryStart = solution->seperators[vehicleIndex-1];
    solution->move(callIndex, temporaryStart+1, temporaryStart+2);
    if (vehicleIndex != vehicleCall) {
        solution->updateCost(vehicleCall);
    }

    // Store all feasible indices
    std::vector<std::pair<int, int>> feasibleIndices;
    int pointer1 = startIndex;
    while (pointer1 < endIndex-1) {
        // Ensure that current start is feasible
        if (times[pointer1-startIndex] > call.pickupWindow.end) {
            break;
        }
        if (capacities[pointer1-startIndex] < call.size) {
            pointer1++;
            continue;
        }

        int pointer2 = pointer1;
        while (++pointer2 < endIndex) {
            // Ensure that current end is feasible
            if (times[pointer2-startIndex] > call.deliveryWindow.end) {
                break;
            }
            // Move call
            solution->greedyMove(callIndex, startIndex, endIndex, pointer1, pointer2);
            solution->invalidateCache();

            // Check feasibility
            solution->updateFeasibility(vehicleIndex);
            if (!solution->isFeasible()) {
                continue;
            }

            // If feasible, store indices 
            feasibleIndices.push_back(std::make_pair(pointer1, pointer2));
        }
        pointer1++;
    }

    // Return all feasible indices
    return feasibleIndices;
}

std::pair<int, std::pair<int, int>> getBestInsertion(int callIndex, int vehicleIndex, Solution* solution) {
    // Create a current copy of the solution
    Solution current = solution->copy();

    // Store best values
    int bestCost = INT_MAX;
    std::pair<int, int> bestIndices;
    
    // Find the vehicle which currently has the call
    int vehicleCall = current.getVehicleWith(callIndex);

    // Get the call
    Call call = current.problem->calls[callIndex-1];

    // Calculate times and capacities of current vehicle
    std::pair<std::vector<int>, std::vector<int>> details = solution->getDetails(vehicleIndex);
    std::vector<int> times = details.first, capacities = details.second;

    // Move call to start of current vehicle as default before starting
    int temporaryStart = current.seperators[vehicleIndex-1];
    current.move(callIndex, temporaryStart+1, temporaryStart+2);
    if (vehicleIndex != vehicleCall) {
        current.updateCost(vehicleCall);
    }

    // Find the best possible different position
    int startIndex = current.seperators[vehicleIndex-1]+1, endIndex = current.seperators[vehicleIndex];
    int pointer1 = startIndex;
    while (pointer1 < endIndex-1) {
        // Ensure that current start is feasible
        if (times[pointer1-startIndex] > call.pickupWindow.end) {
            break;
        }
        if (capacities[pointer1-startIndex] < call.size) {
            pointer1++;
            continue;
        }

        int pointer2 = pointer1;
        while (++pointer2 < endIndex) {
            // Ensure that current end is feasible
            if (times[pointer2-startIndex] > call.deliveryWindow.end) {
                break;
            }
            // Move call
            current.greedyMove(callIndex, startIndex, endIndex, pointer1, pointer2);
            current.invalidateCache();

            // Check feasibility
            current.updateFeasibility(vehicleIndex);
            if (!current.isFeasible()) {
                continue;
            }

            // If feasible, update cost and store if best (and if different)
            current.updateCost(vehicleIndex);
            if (current.getCost() < bestCost) {
                bestCost = current.getCost();
                bestIndices = std::make_pair(pointer1, pointer2);
            }
        }
        pointer1++;
    }

    // Return the best values
    return std::make_pair(bestCost, bestIndices);
}