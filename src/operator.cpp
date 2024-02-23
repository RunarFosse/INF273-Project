#include "operator.h"

Solution OneInsert::apply(Solution solution, std::default_random_engine& rng) {
    // Create a copy of the current solution
   solution = solution.copy();
   
    // Select a random call
    int callIndex = std::uniform_int_distribution<std::size_t>(1, solution.problem->noCalls)(rng);

    // Find vehicle of and remove occurences of callIndex from representation
    int skip = 0;
    int currentVehicle = 1, callVehicle;
    for (int i = 0; i < solution.representation.size(); i++) {
        // Override occurences of callIndex
        while (i+skip < solution.representation.size() && solution.representation[i+skip] == callIndex) {
            skip++;
            // And record which vehicle has the call
            callVehicle = currentVehicle;
        }
        if (i+skip >= solution.representation.size()) {
            break;
        }

        solution.representation[i] = solution.representation[i+skip];

        if (solution.representation[i] == 0) {
            currentVehicle++;
        }
    }

    // Decide if we should outsource or give to our vehicles
    int vehicleIndex;
    double outsourceProbability = 1.0 / (1.2 * (solution.problem->noVehicles+1));
    if (callVehicle != solution.problem->noVehicles+1 && std::uniform_real_distribution<double>(0, 1)(rng) < outsourceProbability) {
        vehicleIndex = solution.problem->noVehicles+1;
    } else {
        // Retrieve the list of possible vehicles for this call
        std::vector<int> possibleVehicles;
        for (int vehicleIndex : solution.problem->calls[callIndex-1].possibleVehicles) {
            if (vehicleIndex == callVehicle) {
                continue;
            }
            possibleVehicles.push_back(vehicleIndex);
        }

        // Then select a different random vehicle to take this call instead
        if (possibleVehicles.empty()) {
            vehicleIndex = solution.problem->noVehicles+1;
        } else {
            vehicleIndex = possibleVehicles[std::uniform_int_distribution<std::size_t>(0, possibleVehicles.size()-1)(rng)];
        }
    }

    // Insert the callIndex under the new vehicleIndex at random positions
    std::deque<int> buffer;
    int insert1 = -1, insert2 = -1;
    currentVehicle = 1;
    for (int i = 0; i < solution.representation.size(); i++) {
        // If we haven't calculated insertion positions for call,
        // and we are at the right vehicle, then do so (special rules for when outsourcing)
        if (currentVehicle == vehicleIndex && insert1 == -1) {
            // Custom insert for when outsourcing
            if (vehicleIndex == solution.problem->noVehicles+1) {
                if (solution.representation[i] > callIndex || i == solution.representation.size()-3) {
                    insert1 = i;
                    insert2 = i+1;
                }
            } else {
                int startIndex = i, endIndex = solution.representation.size();
                for (int j = i; j < endIndex; j++) {
                    if (solution.representation[j] == 0) {
                        endIndex = j+1;
                        break;
                    }
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
        }

        // Insert callIndex if at insertion position,
        // if not, insert from front of buffer
        if (i == insert1 || i == insert2) {
            buffer.push_back(solution.representation[i]);
            solution.representation[i] = callIndex;
        } else if (buffer.size() > 0) {
            buffer.push_back(solution.representation[i]);
            solution.representation[i] = buffer.front();
            buffer.pop_front();
        }

        // Increment current vehicle if seperator is found
        if (solution.representation[i] == 0) {
            currentVehicle++;
        }
    }

    // Return the modified neighbour solution
    return solution;
}
