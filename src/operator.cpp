#include "operator.h"

#include "debug.h"

UniformOperator::UniformOperator(std::vector<Operator*> operators) {
    this->operators = operators;
}

Solution UniformOperator::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Get a random operator from those this contains
    int operatorIndex = std::uniform_int_distribution<std::size_t>(0, this->operators.size()-1)(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, iteration, rng);
}

WeightedOperator::WeightedOperator(std::vector<std::pair<Operator*, double>> operators) {
    for (std::pair<Operator*, double> op : operators) {
        this->operators.push_back(op.first);
        this->weights.push_back(op.second);
    }
}

Solution WeightedOperator::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Get a weighted random operator from those this contains
    int operatorIndex = std::discrete_distribution<std::size_t>(this->weights.begin(), this->weights.end())(rng);

    // Apply it
    return this->operators[operatorIndex]->apply(solution, iteration, rng);
}

AdaptiveOperator::AdaptiveOperator(std::vector<Operator*> operators) {
    this->operators = operators;
    this->weights.resize(operators.size());
    this->scores.resize(operators.size());
    this->uses.resize(operators.size());

    this->reset();
}

Solution AdaptiveOperator::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // If first iteration, reset the weights
    if (iteration == 0) {
        this->reset();
    }

    // else if iterations are a multiple of 100, recalculate adaptive weights
    else if (iteration % 100 == 0) {
        this->update();
    }

    // Get a weighted random operator from those this contains
    int operatorIndex = std::discrete_distribution<std::size_t>(this->weights.begin(), this->weights.end())(rng);
    this->lastOperatorUsed = operatorIndex;

    // Apply it
    Solution newSolution = this->operators[operatorIndex]->apply(solution, iteration, rng);

    // Add a use to the current operator
    this->uses[operatorIndex]++;

    int deltaCost = solution->getCost() - newSolution.getCost();

    // Update scoring based on how good the new solution is
    if (newSolution.getCost() < bestCost) {
        // New best solution
        bestCost = newSolution.getCost();
        this->scores[operatorIndex] += 4;
    } else if (newSolution.getCost() < solution->getCost()) {
        // Better than previous solution
        this->scores[operatorIndex] += 2;
    } else if (this->seenSolutions.find(newSolution) == this->seenSolutions.end()) {
        // New unique solution
        seenSolutions.insert(newSolution);
        this->scores[operatorIndex] += 1;
    }

    // And return it
    return newSolution;
}

void AdaptiveOperator::update() {
    // Store sum for normalization later
    double sum = 0;

    for (int i = 0; i < this->operators.size(); i++) {
        this->weights[i] *= (1.0 - r);

        // Don't add to weight if operator wasn't used
        if (this->uses[i] > 0) {
            this->weights[i] += r * this->scores[i] / (double)this->uses[i];
        }

        // Also ensure weights never become zero
        if (this->weights[i] < 0.01) {
            this->weights[i] = 0.01;
        }

        // Reset score/uses
        this->scores[i] = 0;
        this->uses[i] = 0;

        sum += this->weights[i];
    }

    // And normalize all the weights
    //Debugger::printToTerminal("Weights: [");
    for (int i = 0; i < this->operators.size(); i++) {
        this->weights[i] /= sum;
        //Debugger::printToTerminal(std::to_string(this->weights[i]) + ", ");
    }
    //Debugger::printToTerminal("]\n");
}

void AdaptiveOperator::reset() {
    // Reset weights
    for (int i = 0; i < this->operators.size(); i++) {
        this->weights[i] = 1.0 / this->operators.size();

        // Reset score/uses
        this->scores[i] = 0;
        this->uses[i] = 0;
    }
    // Reset seen solutions
    this->seenSolutions.clear();
}

Solution SimilarGreedyInsert::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = boundedUniformSample(solution, iteration, rng);

    // Remove similar calls
    std::vector<int> removedCalls = removeSimilar(callsToMove, &current, rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertGreedy(callIndices, &current);

    // Return the neighbour solution
    return current;
}

Solution SimilarRegretInsert::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = boundedUniformSample(solution, iteration, rng);

    // Remove similar calls
    std::vector<int> removedCalls = removeSimilar(callsToMove, &current, rng);

    // Infer k from current iteration
    int k = std::uniform_int_distribution<int>(2, 4)(rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertRegret(callIndices, &current, k);

    // Return the neighbour solution
    return current;
}

Solution CostlyGreedyInsert::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = boundedUniformSample(solution, iteration, rng);

    // Remove current most costly calls
    std::vector<int> removedCalls = removeCostly(callsToMove, &current, rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertGreedy(callIndices, &current);

    // Return the neighbour solution
    return current;
}

Solution CostlyRegretInsert::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = boundedUniformSample(solution, iteration, rng);

    // Remove current most costly calls
    std::vector<int> removedCalls = removeCostly(callsToMove, &current, rng);

    // Infer k from current iteration
    int k = std::uniform_int_distribution<int>(2, 4)(rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertRegret(callIndices, &current, k);

    // Return the neighbour solution
    return current;
}


Solution RandomGreedyInsert::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = boundedUniformSample(solution, iteration, rng);

    // Remove random calls
    std::vector<int> removedCalls = removeRandom(callsToMove, &current, rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertGreedy(callIndices, &current);

    // Return the neighbour solution
    return current;
}

Solution RandomRegretInsert::apply(Solution* solution, int iteration, std::default_random_engine& rng) {
    // Create a copy of the current solution
    Solution current = solution->copy();

    // Pick out the number of calls to move
    int callsToMove = boundedUniformSample(solution, iteration, rng);

    // Remove random calls
    std::vector<int> removedCalls = removeRandom(callsToMove, &current, rng);

    // Infer k from current iteration
    int k = std::uniform_int_distribution<int>(2, 4)(rng);

    // Insert them using greedy
    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
    insertRegret(callIndices, &current, k);

    // Return the neighbour solution
    return current;
}

int boundedUniformSample(Solution* solution, int iteration, std::default_random_engine& rng) {
    int lowerbound = std::uniform_int_distribution<int>(1, std::max(1, solution->problem->noCalls / 3))(rng);
    int upperbound = std::max(lowerbound, solution->problem->noCalls / 2);
    return std::uniform_int_distribution<int>(1, upperbound)(rng);
}