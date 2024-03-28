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

Solution RandomInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int lowerbound = 1, upperbound = solution->problem->noCalls;

    double mean = solution->problem->noCalls * 0.2, std = solution->problem->noCalls * 0.075;
    mean = 2.0;
    std = 2.0;
    int callsToInsert = std::clamp((int)std::ceil(std::normal_distribution<double>(mean, std)(rng)), lowerbound, upperbound);

    return *performRandomInsert(callsToInsert, &current, rng);
}

Solution RandomBestInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int lowerbound = 1, upperbound = solution->problem->noCalls;

    double mean = solution->problem->noCalls * 0.2, std = solution->problem->noCalls * 0.1;
    mean = 2.0;
    std = 2.0;
    int callsToInsert = std::clamp((int)std::ceil(std::normal_distribution<double>(mean, std)(rng)), lowerbound, upperbound);

    return *performBestInsert(callsToInsert, false, &current, rng);
}

Solution GreedyBestInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int lowerbound = 1, upperbound = solution->problem->noCalls;

    double mean = solution->problem->noCalls * 0.2, std = solution->problem->noCalls * 0.1;
    mean = 3.0;
    std = 2.0;
    int callsToInsert = std::clamp((int)std::ceil(std::normal_distribution<double>(mean, std)(rng)), lowerbound, upperbound);

    return *performBestInsert(callsToInsert, true, &current, rng);
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
    int upperbound = 3;//std::max(std::min((int)possibleCalls.size(), current.problem->noCalls / 20), 1);
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

Solution* performRandomInsert(int callsToInsert, Solution* solution, std::default_random_engine& rng) {
    // Efficient (non-colliding) sampling algorithm "https://stackoverflow.com/a/3724708"
    std::set<int> callIndices;
    int total = solution->problem->noCalls+1;
    for (int i = total - callsToInsert; i < total; i++) {
        int callIndex = std::uniform_int_distribution<int>(1, i)(rng); 
        if (callIndices.find(callIndex) != callIndices.end()) {
            callIndex = i;
        }
        callIndices.insert(callIndex);

        // And temporarily remove them
        solution->remove(callIndex);
    }

    for (int callIndex : callIndices) {
        // Find all feasible insertions
        std::vector<std::pair<int, CallDetails>> feasibleInsertions = calculateFeasibleInsertions(callIndex, solution, false);

        // Then randomly sample a feasible insertion
        CallDetails insertion = feasibleInsertions[std::uniform_int_distribution<int>(0, feasibleInsertions.size()-1)(rng)].second;

        // Insert the callIndex under the new vehicleIndex at random positions
        solution->add(insertion.vehicle, callIndex, insertion.indices);
    }

    // Return the modified neighbour solution
    return solution;
}

Solution* performBestInsert(int callsToInsert, bool greedy, Solution* solution, std::default_random_engine& rng) {
    // Efficient (non-colliding) sampling algorithm "https://stackoverflow.com/a/3724708"
    std::set<int> callIndices;
    int total = solution->problem->noCalls+1;
    for (int i = total - callsToInsert; i < total; i++) {
        int callIndex = std::uniform_int_distribution<int>(1, i)(rng); 
        if (callIndices.find(callIndex) != callIndices.end()) {
            callIndex = i;
        }
        callIndices.insert(callIndex);

        // And temporarily remove them
        solution->remove(callIndex);
    }

    // Then move each call to the best possible position
    while (!callIndices.empty()) {
        int bestCost = INT_MAX, bestCall;
        CallDetails bestInsertion;

        for (int callIndex : callIndices) {
            // Find all feasible insertions sorted from best-to-worst
            std::vector<std::pair<int, CallDetails>> feasibleInsertions = calculateFeasibleInsertions(callIndex, solution, true);

            // If greedy, store only the best entry of them all
            if (greedy) {
                if (feasibleInsertions[0].first < bestCost) {
                    bestCost = feasibleInsertions[0].first;
                    bestCall = callIndex;
                    bestInsertion = feasibleInsertions[0].second;
                }
            // Else if not, select the first best entry
            } else {
                bestCall = callIndex;
                bestInsertion = feasibleInsertions[0].second;
                break;
            }
        }

        // At end, add callIndex to bestIndices and continue with the rest
        solution->add(bestInsertion.vehicle, bestCall, bestInsertion.indices);
        callIndices.erase(bestCall);
    }

    return solution;
}