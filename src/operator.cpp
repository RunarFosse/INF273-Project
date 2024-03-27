#include "operator.h"

#include "debug.h"

UniformOperator::UniformOperator(std::vector<Operator*> operators) {
    this->operators = operators;
}

Solution UniformOperator::apply(Solution* solution, std::default_random_engine& rng) {
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

Solution WeightedOperator::apply(Solution* solution, std::default_random_engine& rng) {
    // Get a weighted random operator from those this contains
    int operatorIndex = std::discrete_distribution<std::size_t>(this->weights.begin(), this->weights.end())(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, rng);
}

Solution OneInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Select a random call
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution->problem->noCalls)(rng);

    // Find all feasible insertions sorted from best-to-worst
    std::vector<std::pair<int, CallDetails>> feasibleInsertions = calculateFeasibleInsertions(callIndex, &current, false);
    // If no feasible position has been found, keep call outsourced
    if (feasibleInsertions.empty()) {
        return current;
    }

    // Then randomly sample a feasible insertion
    CallDetails insertion = feasibleInsertions[std::uniform_int_distribution<int>(0, feasibleInsertions.size()-1)(rng)].second;

    // Insert the callIndex under the new vehicleIndex at random positions
    current.move(insertion.vehicle, callIndex, insertion.indices);

    // Return the modified neighbour solution
    return current;
}

Solution ConstantBestInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int lowerbound = 1;
    int upperbound = std::min(solution->problem->noCalls, 4);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &current, rng);
}

Solution LowBestInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int lowerbound = 1;
    int upperbound = std::max(solution->problem->noCalls / 8, 1);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &current, rng);
}

Solution HighBestInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int lowerbound = std::max(solution->problem->noCalls / 10, 1);
    int upperbound = std::max(solution->problem->noCalls / 5, 1);
    int callsToInsert = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

    return *performBestInsert(callsToInsert, &current, rng);
}

Solution MultiOutsource::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Extract all currently outsourced calls
    std::set<int> outsourcedCalls;
    for (int callIndex : solution->representation[solution->outsourceVehicle-1]) {
        outsourcedCalls.insert(callIndex);
    }

    // Compute all not-outsourced
    std::vector<int> possibleCalls;
    for (int callIndex = 1; callIndex <= solution->problem->noCalls; callIndex++) {
        if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
            possibleCalls.push_back(callIndex);
        }
    }

    // If no call can be outsourced, return current solution
    if (possibleCalls.empty()) {
        return current;
    }

    // Pick the number random calls to outsource
    int lowerbound = 1;
    int upperbound = std::max(std::min((int)possibleCalls.size(), current.problem->noCalls / 20), 1);
    int callsToOutsource = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);
    
    std::vector<int> callIndices;
    std::sample(possibleCalls.begin(), possibleCalls.end(), std::back_inserter(callIndices), callsToOutsource, rng);

    // Outsource all of them
    for (int callIndex : callIndices) {
        current.outsource(callIndex);
    }

    // Return neighbour solution
    return current;
}


Solution* performBestInsert(int callsToInsert, Solution* solution, std::default_random_engine& rng) {
    // Efficient (non-colliding) sampling algorithm "https://stackoverflow.com/a/3724708"
    std::vector<int> callIndices, allCalls;
    allCalls.reserve(solution->problem->noCalls);
    for (int callIndex = 1; callIndex <= solution->problem->noCalls; callIndex++) {
        allCalls.push_back(callIndex);
    }
    std::sample(allCalls.begin(), allCalls.end(), std::back_inserter(callIndices), callsToInsert, rng);
    std::shuffle(callIndices.begin(), callIndices.end(), rng);

    // Temporarily move all to outsource and update the cost
    for (int callIndex : callIndices) {
        solution->outsource(callIndex);
    }

    // Then move each call to the best possible position
    for (int callIndex : callIndices) {
        // Find all feasible insertions sorted from best-to-worst
        std::vector<std::pair<int, CallDetails>> feasibleInsertions = calculateFeasibleInsertions(callIndex, solution, true);

        // If no feasible position has been found, keep call outsourced
        if (feasibleInsertions.empty()) {
            continue;
        }

        // At end, move callIndex to bestIndices and return new solution
        solution->move(feasibleInsertions[0].second.vehicle, callIndex, feasibleInsertions[0].second.indices);
    }

    return solution;
}

std::vector<std::pair<int, CallDetails>> calculateFeasibleInsertions(int callIndex, Solution* solution, bool sort) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Get the call and its feasible vehicles
    Call& call = current.problem->calls[callIndex-1];
    std::vector<int>& possibleVehicles = call.possibleVehicles;

    // Initialize a vector for storing feasible insertions
    std::vector<std::pair<int, CallDetails>> feasibleInsertions;

    for (int vehicleIndex : possibleVehicles) {
        // Store vehicle details
        std::pair<std::vector<int>, std::vector<int>> details = current.getDetails(vehicleIndex);
        std::vector<int>& times = details.first, capacities = details.second;

        // Remove the call from wherever it is, and add it to start of current vehicle
        current.move(vehicleIndex, callIndex, std::make_pair(0, 1));

        // Store previous cost for easy updates
        int previousCost = current.costs[vehicleIndex-1];

        // Loop over every possible insertion point
        for (int pointer1 = 0; pointer1 < current.representation[vehicleIndex-1].size()-1; pointer1++) {
            // Ensure that current start is feasible
            if (times[pointer1] > call.pickupWindow.end) {
                break;
            }
            if (capacities[pointer1] < call.size) {
                continue;
            }

            int pointer2 = pointer1;
            while (++pointer2 < current.representation[vehicleIndex-1].size()) {
                // Ensure that current end is feasible
                if (times[pointer2] > call.deliveryWindow.end) {
                    break;
                }

                // Move call
                std::pair<int, int> indices = std::make_pair(pointer1, pointer2);
                current.move(vehicleIndex, callIndex, indices);

                // Check feasibility
                current.updateFeasibility(vehicleIndex);
                if (!current.isFeasible()) {
                    continue;
                }

                // If it is feasible, store it
                CallDetails callDetail = {vehicleIndex, indices};
                feasibleInsertions.push_back(std::make_pair(current.getCost(), callDetail));
            }
        }
    }

    // After all insertions, sort the vector by cost in ascending order and return
    if (sort) {
        std::sort(feasibleInsertions.begin(), feasibleInsertions.end(), [](const std::pair<int, CallDetails>& a, const std::pair<int, CallDetails>& b) {
            return a.first < b.first;
        });
    }
    return feasibleInsertions;
}