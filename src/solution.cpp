#include "solution.h"

Solution::Solution(Problem* problem) {
    // Link problem to solution
    this->problem = problem;

    // Reserve representation size
    this->representation.reserve(problem->noVehicles + 2 * problem->noCalls);
}

Solution::Solution(std::vector<int> representation, Problem* problem) {
    // Link problem to solution
    this->problem = problem;

    // Set representation to what is given
    this->representation = representation;
}

Solution Solution::initialSolution(Problem* problem) {
    // Create an empty solution
    Solution solution(problem);

    // Outsource all calls
    for (int i = 0; i < problem->noVehicles; i++) {
        solution.representation.push_back(0);
    }
    for (int callIndex = 1; callIndex <= problem->noCalls; callIndex++) {
        solution.representation.push_back(callIndex);
        solution.representation.push_back(callIndex);
    }

    // Return initial solution
    return solution;
}

Solution Solution::randomSolution(Problem* problem, std::default_random_engine& rng) {
    // Create an empty initial solution
    Solution solution(problem);

    // For each vehicle + outsource, initialize a vector
    std::vector<std::deque<int>> vehicles(problem->noVehicles + 1);

    // For each call
    for (int callIndex = 1; callIndex <= problem->noCalls; callIndex++) {
        // Retrieve the list of possible vehicles for this call
        std::vector<int> possibleVehicles = problem->calls[callIndex-1].possibleVehicles;

        // Pick a random vehicle which can take this call
        std::uniform_int_distribution<std::size_t> distribution(0, possibleVehicles.size()+2);
        int index = distribution(rng);

        // Check if we should outsource the call (3x higher chance)
        if (index >= possibleVehicles.size()) {
            vehicles[vehicles.size()-1].push_back(callIndex);
            vehicles[vehicles.size()-1].push_back(callIndex);
            continue;
        }

        int vehicleIndex = possibleVehicles[index];
        vehicles[vehicleIndex-1].push_back(callIndex);
        vehicles[vehicleIndex-1].push_back(callIndex);
    }

    // Add the calls to the solution representation
    for (int i = 0; i < vehicles.size(); i++) {
        while (!vehicles[i].empty()) {
            solution.representation.push_back(vehicles[i].front());
            vehicles[i].pop_front();
        }

        // Add a 0 if not finished
        if (i < vehicles.size() - 1) {
            solution.representation.push_back(0);
        }
    }

    int retries = 3;
    do {
        // Then shuffle the order of calls in our vehicles (not the outsourced as that is redundant)
        std::vector<int>::iterator vehicleCallsBegin = solution.representation.begin(), vehicleCallsEnd = std::find(vehicleCallsBegin, solution.representation.end(), 0);
        while (vehicleCallsEnd != solution.representation.end()) {
            std::shuffle(vehicleCallsBegin, vehicleCallsEnd, rng);
            vehicleCallsBegin = vehicleCallsEnd+1, vehicleCallsEnd = std::find(vehicleCallsBegin, solution.representation.end(), 0);
        }
        solution.invalidateCache();
        retries--;
    } while (!solution.isFeasible() && retries > 0);

    // Return the randomized solution
    return solution;
}

bool Solution::isFeasible() {
    // Check if value is cached
    if (this->feasibilityCache.first) {
        return this->feasibilityCache.second;
    }

    int i = 0;

    // Handle our vehicles
    for (int vehicleIndex = 1; vehicleIndex <= this->problem->noVehicles; vehicleIndex++) {
        Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

        std::unordered_set<int> startedCalls;
        std::unordered_set<int> possibleCalls(vehicle.possibleCalls.begin(), vehicle.possibleCalls.end());

        int currentTime = vehicle.startTime;
        int currentCapacity = vehicle.capacity;
        int currentNode = vehicle.homeNode;

        while (this->representation[i] != 0) {
            int callIndex = this->representation[i];

            if (possibleCalls.find(callIndex) == possibleCalls.end()) {
                // Vehicle incompatible with call
                this->feasibilityCache = std::make_pair(true, false);
                return this->feasibilityCache.second;
            }

            if (startedCalls.find(callIndex) == startedCalls.end()) {
                startedCalls.insert(callIndex);
                // Pickup call cargo
                Call call = this->problem->calls[callIndex-1];
                
                // Travel to call origin node
                currentTime += vehicle.routeTimeCost[currentNode-1][call.originNode-1].time;
                currentNode = call.originNode;

                // Verify within time window for pickup (inclusive)
                if (currentTime < call.pickupWindow.start) {
                    // Wait if arrived early
                    currentTime = call.pickupWindow.start;
                }
                if (currentTime > call.pickupWindow.end) {
                    // Arrived outside timewindow
                    this->feasibilityCache = std::make_pair(true, false);
                    return this->feasibilityCache.second;
                }

                // Pickup cargo at origin node (wait some time)
                currentTime += vehicle.callTimeCost[callIndex-1].first.time;
                currentCapacity -= call.size;

                // Verify capacity is not exceeded
                if (currentCapacity < 0) {
                    // Capacity exceeded
                    this->feasibilityCache = std::make_pair(true, false);
                    return this->feasibilityCache.second;
                }
                
            } else {
                // Deliver call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call destination node
                currentTime += vehicle.routeTimeCost[currentNode-1][call.destinationNode-1].time;
                currentNode = call.destinationNode;

                // Verify within time window for delivery (inclusive)
                if (currentTime < call.deliveryWindow.start) {
                    // Wait if arrived early
                    currentTime = call.deliveryWindow.start;
                }
                if (currentTime > call.deliveryWindow.end) {
                    // Arrived outside timewindow
                    this->feasibilityCache = std::make_pair(true, false);
                    return this->feasibilityCache.second;
                }

                // Deliver cargo at destination node (wait some time)
                currentTime += vehicle.callTimeCost[callIndex-1].second.time;
                currentCapacity += call.size;
            }
            i++;
        }

        // Verify that all picked up calls were delivered (Only validity check as it is efficient to compute)
        if (currentCapacity != vehicle.capacity) {
            // Did not finish all started calls
            this->feasibilityCache = std::make_pair(true, false);
            return this->feasibilityCache.second;
        }

        i++;
    }

    // We do not need to check outsource calls (feasible if solution is valid)

    // The solution is feasible!
    this->feasibilityCache = std::make_pair(true, true);
    return this->feasibilityCache.second;
}

int Solution::getCost() {
    // Check if value is cached
    if (this->costCache.first) {
        return this->costCache.second;
    }

    int i = 0;
    int totalCost = 0;

    // Handle our vehicles
    for (int vehicleIndex = 1; vehicleIndex <= this->problem->noVehicles; vehicleIndex++) {
        Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

        int currentNode = vehicle.homeNode;
        std::unordered_set<int> startedCalls;

        while (this->representation[i]) {
            int callIndex = this->representation[i];

            if (startedCalls.find(callIndex) == startedCalls.end()) {
                // Pickup call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call origin node
                totalCost += vehicle.routeTimeCost[currentNode-1][call.originNode-1].cost;
                currentNode = call.originNode;

                // Pickup cargo at origin node (wait some time)
                totalCost += vehicle.callTimeCost[callIndex-1].first.cost;

                startedCalls.insert(callIndex);
            } else {
                // Deliver call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call destination node
                totalCost += vehicle.routeTimeCost[currentNode-1][call.destinationNode-1].cost;
                currentNode = call.destinationNode;

                // Deliver cargo at destination node (wait some time)
                totalCost += vehicle.callTimeCost[callIndex-1].second.cost;
            }

            i++;
        }
        i++;
    }

    // Handle outsourced calls
    std::unordered_set<int> outsourcedCalls;
    while (i < this->representation.size()) {
        int callIndex = this->representation[i];

        // Only count outsourced calls once (for effiency)
        if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
            // Outsource the call
            Call call = this->problem->calls[callIndex-1];
            totalCost += call.costOfNotTransporting;

            outsourcedCalls.insert(callIndex);
        }

        i++;
    }

    // Cache and return the computed cost
    this->costCache = std::make_pair(true, totalCost);
    return this->costCache.second;
}

void Solution::invalidateCache() {
    this->feasibilityCache.first = false;
    this->costCache.first = false;
}