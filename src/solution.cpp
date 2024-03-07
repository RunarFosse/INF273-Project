#include "solution.h"

#include "debug.h"

Solution::Solution(Problem* problem) {
    // Link problem to solution
    this->problem = problem;

    // Reserve representation size
    this->representation.reserve(problem->noVehicles + 2 * problem->noCalls);
    this->seperators.reserve(problem->noVehicles+1);
    this->costs.resize(problem->noVehicles+1);
}

Solution::Solution(std::vector<int> representation, Problem* problem) {
    // Link problem to solution
    this->problem = problem;

    // Set representation to what is given
    this->representation = representation;

    // Find the different seperators in the given solution
    this->seperators.push_back(-1);
    std::vector<int>::iterator pointer1 = representation.begin(), pointer2 = std::find(pointer1, representation.end(), 0);
    while (pointer2 != representation.end()) {
        this->seperators.push_back(std::distance(representation.begin(), pointer2));
        pointer1 = pointer2+1, pointer2 = std::find(pointer1, representation.end(), 0);
    }
}

Solution Solution::copy() {
    // Create a new solution with the same problem
    Solution solution = Solution(this->problem);

    // Copy over representation, seperator and cost vectors
    solution.representation = this->representation;
    solution.seperators = this->seperators;
    solution.costs = this->costs;

    // Copy over feasibility and cost
    solution.feasibilityCache = std::make_pair(true, this->isFeasible());
    solution.costCache = std::make_pair(true, this->getCost());

    // Return copy
    return solution;
}

Solution Solution::initialSolution(Problem* problem) {
    // Create an empty solution
    Solution solution(problem);

    // Outsource all calls
    solution.seperators.push_back(-1);
    for (int i = 0; i < problem->noVehicles; i++) {
        solution.representation.push_back(0);
        solution.seperators.push_back(i);
    }
    for (int callIndex = 1; callIndex <= problem->noCalls; callIndex++) {
        solution.representation.push_back(callIndex);
        solution.representation.push_back(callIndex);
    }

    // Set solution to feasible
    solution.feasibilityCache = std::make_pair(true, true);

    // And then precalcuate cost for outsource
    solution.updateCost(solution.problem->noVehicles+1);

    // Return initial solution
    return solution;
}

Solution Solution::randomSolution(Problem* problem, std::default_random_engine& rng) {
    // Create an empty initial solution
    Solution solution(problem);

    // For each vehicle + outsource, initialize a vector
    std::vector<std::vector<int>> vehicles(problem->noVehicles + 1);

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
    solution.seperators.push_back(-1);
    for (int i = 0; i < vehicles.size(); i++) {
        // Then shuffle the order of calls in our vehicles (not the outsourced as that is redundant)
        bool isNotOutsourcing = i < vehicles.size() - 1;
        if (isNotOutsourcing) {
            std::shuffle(vehicles[i].begin(), vehicles[i].end(), rng);
        }
        for (int callIndex : vehicles[i]) {
            solution.representation.push_back(callIndex);
        }

        // Add a 0 if not finished
        if (isNotOutsourcing) {
            solution.seperators.push_back(solution.representation.size());
            solution.representation.push_back(0);
        }
    }

    // Precalculate cost
    solution.getCost();

    // Return the randomized solution
    return solution;
}

void Solution::greedyMove(int callIndex, int from, int to, int index1, int index2) {
    // TODO: Make one-pass algorithm

    int skip = 0;
    for (int i = from; i+skip <= to; i++) {
        while (i+skip < this->representation.size() && this->representation[i+skip] == callIndex) {
            skip++;
        }
        if (i+skip < this->representation.size()) {
            this->representation[i] = this->representation[i+skip];
        }
    }

    std::deque<int> buffer;
    int currentVehicle = std::distance(this->seperators.begin(), std::upper_bound(this->seperators.begin(), this->seperators.end(), from));
    for (int i = from; i <= to; i++) {
        if (i == index1 || i == index2) {
            buffer.push_back(this->representation[i]);
            this->representation[i] = callIndex;
        } else if (!buffer.empty()) {
            buffer.push_back(this->representation[i]);
            this->representation[i] = buffer.front();
            buffer.pop_front();
        }

        if (this->representation[i] == 0) {
            currentVehicle++;
            this->seperators[currentVehicle-1] = i;
        }
    }
}

void Solution::move(int callIndex, int index1, int index2) {
    // TODO: Make one-pass algorithm

    int skip = 0;
    for (int i = 0; i+skip < this->representation.size(); i++) {
        while (i+skip < this->representation.size() && this->representation[i+skip] == callIndex) {
            skip++;
        }
        if (i+skip < this->representation.size()) {
            this->representation[i] = this->representation[i+skip];
        }
    }

    std::deque<int> buffer;
    int currentVehicle = 1;
    for (int i = 0; i < this->representation.size(); i++) {
        if (i == index1 || i == index2) {
            buffer.push_back(this->representation[i]);
            this->representation[i] = callIndex;
        } else if (!buffer.empty()) {
            buffer.push_back(this->representation[i]);
            this->representation[i] = buffer.front();
            buffer.pop_front();
        }

        if (this->representation[i] == 0) {
            currentVehicle++;
            this->seperators[currentVehicle-1] = i;
        }
    }
}

std::pair<int, int> Solution::outsource(int callIndex) {
    // First find insertion position
    int insertion;
    for (int i = this->representation.size()-1; i >= 0; i--) {
        if (this->representation[i] <= callIndex) {
            insertion = i-1;
            break;
        }
    }

    // Then move call to those positions
    this->move(callIndex, insertion, insertion+1);

    // Return insertion indices
    return std::make_pair(insertion, insertion+1);
}

int Solution::getVehicleWith(int callIndex) {
    // Search all vehicles which can have callIndex
    for (int vehicleIndex : this->problem->calls[callIndex-1].possibleVehicles) {
        int startIndex = this->seperators[vehicleIndex-1]+1, endIndex = this->seperators[vehicleIndex];
        for (int i = startIndex; i < endIndex; i++) {
            if (this->representation[i] == callIndex) {
                return vehicleIndex;
            }
        }
    }

    // If not, search outsourced
    for (int i = this->representation.size()-1; i >= 0; i--) {
        if (this->representation[i] == callIndex) {
            return this->problem->noVehicles+1;
        } else if (this->representation[i] == 0) {
            break;
        }
    }

    // Can't find call in feasible vehicles, throw error
    assert(false);
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

void Solution::updateFeasibility(int vehicleIndex) {
    // Early return as outsource is always feasible
    if (vehicleIndex == this->problem->noVehicles+1) {
        this->feasibilityCache = std::make_pair(true, true);
        return;
    }  

    int i = this->seperators[vehicleIndex-1]+1;
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
            return;
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
                return;
            }

            // Pickup cargo at origin node (wait some time)
            currentTime += vehicle.callTimeCost[callIndex-1].first.time;
            currentCapacity -= call.size;

            // Verify capacity is not exceeded
            if (currentCapacity < 0) {
                // Capacity exceeded
                this->feasibilityCache = std::make_pair(true, false);
                return;
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
                return;
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
        return;
    }

    // The solution is feasible!
    this->feasibilityCache = std::make_pair(true, true);
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
        // Reset cost before computing
        costs[vehicleIndex-1] = 0;

        Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

        int currentNode = vehicle.homeNode;
        std::unordered_set<int> startedCalls;

        while (this->representation[i]) {
            int callIndex = this->representation[i];

            if (startedCalls.find(callIndex) == startedCalls.end()) {
                // Pickup call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call origin node
                costs[vehicleIndex-1] += vehicle.routeTimeCost[currentNode-1][call.originNode-1].cost;
                currentNode = call.originNode;

                // Pickup cargo at origin node (wait some time)
                costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].first.cost;

                startedCalls.insert(callIndex);
            } else {
                // Deliver call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call destination node
                costs[vehicleIndex-1] += vehicle.routeTimeCost[currentNode-1][call.destinationNode-1].cost;
                currentNode = call.destinationNode;

                // Deliver cargo at destination node (wait some time)
                costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].second.cost;
            }

            i++;
        }
        totalCost += costs[vehicleIndex-1];
        i++;
    }


    // Handle outsourced calls
    std::unordered_set<int> outsourcedCalls;
    costs[problem->noVehicles] = 0;
    while (i < this->representation.size()) {
        int callIndex = this->representation[i];

        // Only count outsourced calls once (for effiency)
        if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
            // Outsource the call
            Call call = this->problem->calls[callIndex-1];
            costs[problem->noVehicles] += call.costOfNotTransporting;

            outsourcedCalls.insert(callIndex);
        }

        i++;
    }
    totalCost += costs[problem->noVehicles];

    // Cache and return the computed cost
    this->costCache = std::make_pair(true, totalCost);
    return this->costCache.second;
}

void Solution::updateCost(int vehicleIndex) {
    // Store previous cost temporarily (and clear current)
    int previousCost = this->costs[vehicleIndex-1];
    this->costs[vehicleIndex-1] = 0;

    int i = this->seperators[vehicleIndex-1]+1;

    // Outsource calculation is a little easier
    if (vehicleIndex == this->problem->noVehicles+1) {
        std::unordered_set<int> outsourcedCalls;
        while (i < this->representation.size()) {
            int callIndex = this->representation[i];

            // Only count outsourced calls once (for effiency)
            if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
                // Outsource the call
                Call call = this->problem->calls[callIndex-1];
                costs[vehicleIndex-1] += call.costOfNotTransporting;

                outsourcedCalls.insert(callIndex);
            }

            i++;
        }
    } else {
        Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

        int currentNode = vehicle.homeNode;
        std::unordered_set<int> startedCalls;

        while (this->representation[i] != 0) {
            int callIndex = this->representation[i];

            if (startedCalls.find(callIndex) == startedCalls.end()) {
                // Pickup call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call origin node
                costs[vehicleIndex-1] += vehicle.routeTimeCost[currentNode-1][call.originNode-1].cost;
                currentNode = call.originNode;

                // Pickup cargo at origin node (wait some time)
                costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].first.cost;

                startedCalls.insert(callIndex);
            } else {
                // Deliver call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call destination node
                costs[vehicleIndex-1] += vehicle.routeTimeCost[currentNode-1][call.destinationNode-1].cost;
                currentNode = call.destinationNode;

                // Deliver cargo at destination node (wait some time)
                costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].second.cost;
            }

            i++;
        }
    }

    // Update and cache the new cost
    int newCost = this->costCache.second - previousCost + costs[vehicleIndex-1];
    this->costCache = std::make_pair(true, newCost);
}

std::pair<std::vector<int>, std::vector<int>> Solution::getDetails(int vehicleIndex) {
    std::vector<int> times, capacities;

    int i = this->seperators[vehicleIndex-1]+1;
    Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

    std::unordered_set<int> startedCalls;

    int currentTime = vehicle.startTime;
    int currentCapacity = vehicle.capacity;
    int currentNode = vehicle.homeNode;

    times.push_back(currentTime);
    capacities.push_back(currentCapacity);

    while (this->representation[i] != 0) {
        int callIndex = this->representation[i];

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

            // Pickup cargo at origin node (wait some time)
            currentTime += vehicle.callTimeCost[callIndex-1].first.time;
            currentCapacity -= call.size;
            
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

            // Deliver cargo at destination node (wait some time)
            currentTime += vehicle.callTimeCost[callIndex-1].second.time;
            currentCapacity += call.size;
        }

        times.push_back(currentTime);
        capacities.push_back(currentCapacity);
        i++;
    }

    // Return the details
    return std::make_pair(times, capacities);
}

void Solution::invalidateCache() {
    this->feasibilityCache.first = false;
    this->costCache.first = false;
}