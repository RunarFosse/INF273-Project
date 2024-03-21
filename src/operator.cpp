#include "operator.h"

#include "debug.h"

UniformOperator::UniformOperator(std::vector<Operator*> operators) {
    this->operators = operators;
}

ObsoleteSolution UniformOperator::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Get a random operator from those this contains
    int operatorIndex = std::uniform_int_distribution<std::size_t>(0, this->operators.size()-1)(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, rng);
}

WeightedOperator::WeightedOperator(std::vector<std::pair<Operator*, double>> operators) {
    for (std::pair<Operator*, double> op : operators) {
        this->operators.push_back(op.first);
        this->weights.push_back(op.second);
    }
}

ObsoleteSolution WeightedOperator::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Get a weighted random operator from those this contains
    int operatorIndex = std::discrete_distribution<std::size_t>(this->weights.begin(), this->weights.end())(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, rng);
}

ObsoleteSolution OneInsert::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    ObsoleteSolution current = solution.copy();

    // Select a random call
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noCalls)(rng);
    int vehicleIndex = solution.problem->calls[callIndex-1].possibleVehicles[std::uniform_int_distribution<std::size_t>(0, solution.problem->calls[callIndex-1].possibleVehicles.size()-1)(rng)];

    // Find the vehicle which currently has the call, and its current indices
    int vehicleCall = current.getVehicleWith(callIndex);
    std::pair<int, int> indicesCall = current.callDetails[callIndex-1];

    // Get all feasible insertion indices
    std::vector<std::pair<int, int>> feasibleIndices = getFeasibleInsertions(callIndex, vehicleIndex, &current);

    // If no feasible insertions exist, return unmodified solution
    if (feasibleIndices.empty()) {
        return solution;
    }

    // Then randomly sample two feasible indices
    std::pair<int, int> indices = feasibleIndices[std::uniform_int_distribution<std::size_t>(0, feasibleIndices.size()-1)(rng)];

    // Insert the callIndex under the new vehicleIndex at random positions
    current.move(callIndex, indices.first, indices.second);

    // Invalidate the caches as we've modified the representation
    current.invalidateCache();

    // Then greedily update the feasibility for modified vehicles
    current.updateFeasibility(vehicleIndex);

    //Debugger::printToTerminal("\nRemove in: " + std::to_string(vehicleCall) + ", " + std::to_string(indicesCall.first) + " - " + std::to_string(indicesCall.second) + "\n");
    //Debugger::printToTerminal("Insert in: " + std::to_string(vehicleIndex) + ", " + std::to_string(indices.first) + " - " + std::to_string(indices.second) + "\n");
    //Debugger::printSolution(&current);

    // Also greedily update the cost for the same vehicles (if not inserted in same position)
    if (indicesCall != indices) {
        //Debugger::printToTerminal("Running removal:");
        current.updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, false, &solution);
        //Debugger::printToTerminal("!!\nRunning insertion:");
        current.updateCost(vehicleIndex, callIndex, indices.first, indices.second, true, &solution);
        //Debugger::printToTerminal("!!\n");
    }

    // Return the modified neighbour solution
    return current;
}

ObsoleteSolution GreedyInsert::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Create a current copy of the solution
    ObsoleteSolution current = solution.copy();

    // Select a random call and feasible vehicle
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noCalls)(rng);
    int vehicleIndex = solution.problem->calls[callIndex-1].possibleVehicles[std::uniform_int_distribution<std::size_t>(0, solution.problem->calls[callIndex-1].possibleVehicles.size()-1)(rng)];

    // Find the vehicle which currently has the call and its current indices
    int vehicleCall = current.getVehicleWith(callIndex);
    std::pair<int, int> indicesCall = current.callDetails[callIndex-1];

    // Get the best values
    std::pair<int, std::pair<int, int>> bestInsertion = getBestInsertion(callIndex, vehicleIndex, vehicleCall, &solution, rng);
    int bestCost = bestInsertion.first;
    std::pair<int, int> bestIndices = bestInsertion.second;

    // If no feasible solution has been found, return the solution unmodified
    if (bestCost == INT_MAX) {
        return solution;
    }

    // At end, move callIndex to bestIndices and return new solution
    current.move(callIndex, bestIndices.first, bestIndices.second);
    current.updateCost(vehicleIndex, callIndex, bestIndices.first, bestIndices.second, true, &solution);
    current.updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, false, &solution);

    current.invalidateCache();

    return current;
}

ObsoleteSolution ConstantBestInsert::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Pick out the number of calls to move
    int lowerbound = 1;
    int upperbound = std::min(solution.problem->noCalls, 10);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &solution, rng);
}

ObsoleteSolution LowBestInsert::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Pick out the number of calls to move
    int lowerbound = 1;
    int upperbound = std::max(solution.problem->noCalls / 10, 1);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &solution, rng);
}

ObsoleteSolution HighBestInsert::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Pick out the number of calls to move
    int lowerbound = std::max(solution.problem->noCalls / 20, 1);
    int upperbound = std::max(solution.problem->noCalls / 5, 1);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &solution, rng);
}

ObsoleteSolution MultiOutsource::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
    // Extract all currently outsourced calls
    std::set<int> outsourcedCalls;
    for (int i = solution.representation.size()-1; i >= 0; i -= 2) {
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

    // Pick the number random calls to outsource
    int lowerbound = 1;
    int upperbound = std::max(std::min((int)possibleCalls.size(), solution.problem->noCalls / 8), 1);
    int callsToOutsource = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    // Efficient (non-colliding) sampling algorithm "https://stackoverflow.com/a/3724708"
    std::set<int> callIndices;
    int total = possibleCalls.size();
    for (int i = total - callsToOutsource; i < total; i++) {
        int item = std::uniform_int_distribution<int>(0, i)(rng); 
        if (callIndices.find(item) == callIndices.end()) {
            callIndices.insert(possibleCalls[item]);
        } else {
            callIndices.insert(possibleCalls[i]);
        }
    }

    // Temporarily move all to outsource and update the cost
    for (int callIndex : callIndices) {
        // Find the vehicle with call
        int vehicleCall = solution.getVehicleWith(callIndex);

        // Outsource the call and update costs
        std::pair<int, int> indicesCall = solution.callDetails[callIndex-1];
        solution.updateCost(solution.problem->noVehicles+1, callIndex, -1, -1, true, &solution);
        solution.updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, false, &solution);
        solution.outsource(callIndex);
    }

    // Return neighbour solution
    return solution;
}

ObsoleteSolution GreedyOutsource::apply(ObsoleteSolution solution, std::default_random_engine& rng) {
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
    int bestCost = INT_MAX;
    int bestCall, bestVehicleCall;
    std::pair<int, int> bestIndicesCall;
    for (int callIndex : possibleCalls) {

        int vehicleCall = solution.getVehicleWith(callIndex);
        std::pair<int, int> indicesCall = solution.callDetails[callIndex-1];

        // We check how the cost would update without actually updating the solution itself (that explains the -1s)
        solution.updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, false, &solution);
        solution.updateCost(solution.problem->noVehicles+1, callIndex, -1, -1, true, &solution);

        if (solution.getCost() < bestCost) {
            bestCost = solution.getCost();
            bestCall = callIndex;
            bestVehicleCall = vehicleCall;
            bestIndicesCall = indicesCall;
        }

        solution.updateCost(solution.problem->noVehicles+1, callIndex, -1, -1, false, &solution);
        solution.updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, true, &solution);
    }

    // Outsource it and return new solution
    ObsoleteSolution current = solution.copy();
    std::pair<int, int> bestIndices = current.outsource(bestCall);
    current.updateCost(bestVehicleCall, bestCall, bestIndicesCall.first, bestIndicesCall.second, false, &solution);
    current.updateCost(solution.problem->noVehicles+1, bestCall, bestIndices.first, bestIndices.second, true, &solution);
    return current;
}

std::vector<std::pair<int, int>> getFeasibleInsertions(int callIndex, int vehicleIndex, ObsoleteSolution* solution) {
    // Create a copy of the current solution
    ObsoleteSolution current = solution->copy();

    // Find the call and vehicle which currently has the call
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
    current.move(callIndex, temporaryStart+1, temporaryStart+2);

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
            current.greedyMove(callIndex, startIndex, endIndex, pointer1, pointer2);
            current.invalidateCache();

            // Check feasibility
            current.updateFeasibility(vehicleIndex);
            if (!current.isFeasible()) {
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

std::pair<int, std::pair<int, int>> getBestInsertion(int callIndex, int vehicleIndex, int vehicleCall, ObsoleteSolution* solution, std::default_random_engine& rng) {
    // Create a current copy of the solution
    ObsoleteSolution current = solution->copy();

    // Store best values
    int bestCost = INT_MAX;
    std::pair<int, int> bestIndices;
    
    // Find the indices of where the call currently is
    std::pair<int, int> indicesCall = current.callDetails[callIndex-1];

    // Get the call
    Call call = current.problem->calls[callIndex-1];

    // Calculate times and capacities of current vehicle
    std::pair<std::vector<int>, std::vector<int>> details = solution->getDetails(vehicleIndex);
    std::vector<int> times = details.first, capacities = details.second;

    // Move call to start of current vehicle as default before starting
    int temporaryStart = current.seperators[vehicleIndex-1];
    current.move(callIndex, temporaryStart+1, temporaryStart+2);
    current.updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, false, solution);

    // Unmodified cost
    int temporaryCost = current.costs[vehicleIndex-1];

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

            // If feasible, update cost and store if best (and ensure it is unmodified from last iteration)
            current.costs[vehicleIndex-1] = temporaryCost;
            current.updateCost(vehicleIndex, callIndex, pointer1, pointer2, true, solution);
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

ObsoleteSolution* performBestInsert(int callsToInsert, ObsoleteSolution* solution, std::default_random_engine& rng) {
    // Efficient (non-colliding) sampling algorithm "https://stackoverflow.com/a/3724708"
    std::set<int> callIndicesSet;
    std::vector<int> callIndices;
    int total = solution->problem->noCalls+1;
    for (int i = total - callsToInsert; i < total; i++) {
        int item = std::uniform_int_distribution<int>(1, i)(rng); 
        if (callIndicesSet.find(item) == callIndicesSet.end()) {
            callIndicesSet.insert(item);
            callIndices.push_back(item);
        } else {
            callIndicesSet.insert(i);
            callIndices.push_back(i);
        }
    }

    // Temporarily move all to outsource and update the cost
    //Debugger::printToTerminal("Moving ");
    for (int callIndex : callIndices) {
        // Find the vehicle with call
        int vehicleCall = solution->getVehicleWith(callIndex);

        //Debugger::printToTerminal(std::to_string(callIndex) + ", ");

        // Outsource the call and update costs
        std::pair<int, int> indicesCall = solution->callDetails[callIndex-1];
        solution->updateCost(solution->problem->noVehicles+1, callIndex, -1, -1, true, solution);
        solution->updateCost(vehicleCall, callIndex, indicesCall.first, indicesCall.second, false, solution);
        solution->outsource(callIndex);
    }
    //Debugger::printToTerminal("\n");

    // Then move each call to the best possible position
    for (int callIndex : callIndices) {
        // Store best values
        int bestCost = INT_MAX;
        int bestVehicle;
        std::pair<int, int> bestIndices;

        // Get all possible vehicles to insert callIndex into
        std::unordered_set<int>* possibleVehicles = &(solution->problem->calls[callIndex-1].possibleVehiclesSet);

        // Get the vehicle which currently has the call (currently all are outsourced so this is easy)
        int vehicleCall = solution->problem->noVehicles+1;

        // Check to place within any (feasible) vehicle (not outsource)
        for (int vehicleIndex = 1; vehicleIndex <= solution->problem->noVehicles; vehicleIndex++) {
            if (possibleVehicles->find(vehicleIndex) == possibleVehicles->end()) {
                continue;
            }
            // Get the best place to insert
            std::pair<int, std::pair<int, int>> bestInsertion = getBestInsertion(callIndex, vehicleIndex, vehicleCall, solution, rng);

            if (bestInsertion.first < bestCost) {
                bestCost = bestInsertion.first;
                bestVehicle = vehicleIndex;
                bestIndices = bestInsertion.second;
            }
        }

        // If nothing has been found, keep call outsourced
        if (bestCost == INT_MAX) {
            continue;
        }

        // At end, move callIndex to bestIndices and return new solution
        solution->move(callIndex, bestIndices.first, bestIndices.second);
        solution->updateCost(bestVehicle, callIndex, bestIndices.first, bestIndices.second, true, solution);
        solution->updateCost(solution->problem->noVehicles+1, callIndex, -1, -1, false, solution);
    }

    return solution;
}