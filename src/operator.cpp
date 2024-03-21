#include "operator.h"

#include "debug.h"

UniformOperator::UniformOperator(std::vector<Operator*> operators) {
    this->operators = operators;
}

Solution UniformOperator::apply(Solution solution, std::default_random_engine& rng) {
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

Solution WeightedOperator::apply(Solution solution, std::default_random_engine& rng) {
    // Get a weighted random operator from those this contains
    int operatorIndex = std::discrete_distribution<std::size_t>(this->weights.begin(), this->weights.end())(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, rng);
}

Solution BestInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Pick out the number of calls to move
    int lowerbound = 1;
    int upperbound = std::max(solution.problem->noCalls / 5, 1);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &solution, rng);
}

Solution MultiOutsource::apply(Solution solution, std::default_random_engine& rng) {
    // Extract all currently outsourced calls
    std::set<int> outsourcedCalls;
    for (int callIndex : solution.representation[solution.outsourceVehicle-1]) {
        outsourcedCalls.insert(callIndex);
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
    int upperbound = std::max(std::min((int)possibleCalls.size(), solution.problem->noCalls / 15), 1);
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

    // Outsource all of them
    for (int callIndex : callIndices) {
        solution.outsource(callIndex);
    }

    // Return neighbour solution
    return solution;
}

std::pair<int, std::pair<int, int>> getBestInsertion(int callIndex, int vehicleIndex, int vehicleCall, Solution* solution, std::default_random_engine& rng) {
    // Create a current copy of the solution
    Solution current = solution->copy();

    // Store best values
    int bestCost = INT_MAX;
    std::pair<int, int> bestIndices;
    
    // Find the indices of where the call currently is
    std::pair<int, int> indicesCall = current.callDetails[callIndex-1].indices;

    // Get the call
    Call call = current.problem->calls[callIndex-1];

    // Calculate times and capacities of current vehicle
    std::pair<std::vector<int>, std::vector<int>> details = solution->getDetails(vehicleIndex);
    std::vector<int> times = details.first, capacities = details.second;

    // Move call to start of current vehicle as default before starting
    current.move(vehicleIndex, callIndex, std::make_pair(0, 1));

    // Unmodified cost
    int temporaryCost = current.costs[vehicleIndex-1];

    // Find the best possible different position
    int endIndex = current.representation[vehicleIndex-1].size();
    int pointer1 = 0;
    while (pointer1 < endIndex-1) {
        // Ensure that current start is feasible
        if (times[pointer1] > call.pickupWindow.end) {
            break;
        }
        if (capacities[pointer1] < call.size) {
            pointer1++;
            continue;
        }

        int pointer2 = pointer1;
        while (++pointer2 < endIndex) {
            // Ensure that current end is feasible
            if (times[pointer2] > call.deliveryWindow.end) {
                break;
            }

            // Move call
            current.move(vehicleIndex, callIndex, std::make_pair(pointer1, pointer2));
            current.invalidateCache();


            // Check feasibility
            current.updateFeasibility(vehicleIndex);
            if (!current.isFeasible()) {
                continue;
            }

            // If feasible, update cost and store if best (and ensure it is unmodified from last iteration)
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

Solution* performBestInsert(int callsToInsert, Solution* solution, std::default_random_engine& rng) {
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
    for (int callIndex : callIndices) {
        solution->outsource(callIndex);
    }

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
        solution->move(bestVehicle, callIndex, bestIndices);
    }

    return solution;
}