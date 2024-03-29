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

Solution SimilarGreedyInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = normalSample(3.0, 2.0, &current, rng);

    // Remove similar calls
    std::vector<int> removedCalls = removeSimilar(callsToMove, &current, rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertGreedy(callIndices, &current);

    // Return the neighbour solution
    return current;
}

Solution SimilarRegretInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = normalSample(3.0, 2.0, &current, rng);

    // Remove similar calls
    std::vector<int> removedCalls = removeSimilar(callsToMove, &current, rng);

    // TODO: Make k parameter
    int k = 3;

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertRegret(callIndices, &current, k);

    // Return the neighbour solution
    return current;
}

Solution CostlyGreedyInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = normalSample(3.0, 2.0, &current, rng);

    // Remove current most costly calls
    std::vector<int> removedCalls = removeCostly(callsToMove, &current, rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertGreedy(callIndices, &current);

    // Return the neighbour solution
    return current;
}

Solution CostlyRegretInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = normalSample(3.0, 2.0, &current, rng);

    // Remove current most costly calls
    std::vector<int> removedCalls = removeCostly(callsToMove, &current, rng);

    // TODO: Make k parameter
    int k = 3;

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertRegret(callIndices, &current, k);

    // Return the neighbour solution
    return current;
}


Solution RandomGreedyInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = normalSample(3.0, 2.0, &current, rng);

    // Remove random calls
    std::vector<int> removedCalls = removeRandom(callsToMove, &current, rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertGreedy(callIndices, &current);

    // Return the neighbour solution
    return current;
}

Solution RandomRegretInsert::apply(Solution* solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = normalSample(3.0, 2.0, &current, rng);

    // Remove random calls
    std::vector<int> removedCalls = removeRandom(callsToMove, &current, rng);

    // TODO: Make k parameter
    int k = 3;

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertRegret(callIndices, &current, k);

    // Return the neighbour solution
    return current;
}


int normalSample(double mean, double std, Solution* solution, std::default_random_engine& rng) {
    // Randomly sample an integer from the given normal distribution
    int sample = std::ceil(std::normal_distribution<double>(mean, std)(rng));

    // Clamp the sampled value to the solution's interval
    int lowerbound = 1, upperbound = solution->problem->noCalls;
    if (sample < lowerbound) {
        return lowerbound;
    }
    if (sample > upperbound) {
        return upperbound;
    }

    return sample;
}