#include "solution.h"

#include "debug.h"

Solution::Solution(Problem* problem) {
    // Link problem to solution
    this->problem = problem;

    // Reserve representation size
    this->representation.resize(problem->noVehicles+1);
    this->costs.resize(problem->noVehicles+1);

    this->callDetails.resize(problem->noCalls);

    // Denote outsource vehicleIndex
    this->outsourceVehicle = problem->noVehicles+1;
}

Solution::Solution(std::vector<int> representation, Problem* problem) : outsourceVehicle(problem->noVehicles+1) {
    // Link problem to solution
    this->problem = problem;

    // Denote outsource vehicleIndex
    this->outsourceVehicle = problem->noVehicles+1;

    // Infer representation from what is given
    int currentVehicle = 1;
    for (int callIndex : representation) {
        if (callIndex == 0) {
            currentVehicle++;
            continue;
        }

        this->representation[currentVehicle-1].push_back(callIndex);
    }

    // Precompute feasibility
    this->isFeasible();
}

Solution Solution::copy() {
    // Create a new solution with the same problem
    Solution solution = Solution(this->problem);

    // Copy over representation, seperator and cost vectors
    solution.representation = this->representation;
    solution.costs = this->costs;
    solution.callDetails = this->callDetails;

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
    int vehicleIndex = solution.outsourceVehicle;
    for (int callIndex = 1; callIndex <= problem->noCalls; callIndex++) {
        // Add callDetail
        int currentIndex = solution.representation[vehicleIndex-1].size();
        solution.callDetails[callIndex-1] = {vehicleIndex, std::make_pair(currentIndex, currentIndex+1)};

        solution.representation[vehicleIndex-1].push_back(callIndex);
        solution.representation[vehicleIndex-1].push_back(callIndex);
    }

    // Set solution to feasible
    solution.isFeasible();

    // And then precalcuate cost
    solution.getCost();

    // Return initial solution
    return solution;
}

Solution Solution::randomSolution(Problem* problem, std::default_random_engine& rng) {
    // Create an empty initial solution
    Solution solution(problem);

    // For each call
    for (int callIndex = 1; callIndex <= problem->noCalls; callIndex++) {
        // Retrieve the list of possible vehicles for this call
        std::vector<int> possibleVehicles = problem->calls[callIndex-1].possibleVehicles;

        // Pick a random vehicle which can take this call
        std::uniform_int_distribution<std::size_t> distribution(0, possibleVehicles.size()+2);
        int index = distribution(rng);

        // Check if we should outsource the call (3x higher chance)
        int vehicleIndex;
        if (index >= possibleVehicles.size()) {
            vehicleIndex = solution.outsourceVehicle;
        } else {
            vehicleIndex = possibleVehicles[index];
        }

        // Initialize callDetails
        int currentIndex = solution.representation[vehicleIndex-1].size();
        solution.callDetails[callIndex-1] = {vehicleIndex, std::make_pair(currentIndex, currentIndex+1)};

        solution.representation[solution.outsourceVehicle].push_back(callIndex);
        solution.representation[solution.outsourceVehicle].push_back(callIndex);
    }

    // At last, shuffle all but outsource vehicle
    for (int vehicleIndex = 1; vehicleIndex < solution.outsourceVehicle; vehicleIndex++) {
        std::shuffle(solution.representation[vehicleIndex-1].begin(), solution.representation[vehicleIndex-1].end(), rng);

        // For each call, add callDetail
        std::unordered_set<int> pickedCalls;
        for (int i = 0; i < solution.representation[vehicleIndex-1].size(); i++) {
            int callIndex = solution.representation[vehicleIndex-1][i];

            if (pickedCalls.find(callIndex) == pickedCalls.end()) {
                solution.callDetails[callIndex-1].indices.first = i;
                pickedCalls.insert(callIndex);
            } else {
                solution.callDetails[callIndex-1].indices.second = i;
            }
        }
    }

    // Precalculate cost and feasibility
    solution.isFeasible();
    solution.getCost();

    // Return the randomized solution
    return solution;
}

void Solution::add(int vehicleIndex, int callIndex, std::pair<int, int> indices) {
    // Resize the vector up
    this->representation[vehicleIndex-1].resize(this->representation[vehicleIndex-1].size()+2);
    
    // Add call to representation
    std::deque<int> buffer;
    std::unordered_set<int> pickedCalls;
    for (int i = 0; i < this->representation[vehicleIndex-1].size(); i++) {
        if (i == indices.first || i == indices.second) {
            buffer.push_back(this->representation[vehicleIndex-1][i]);
            this->representation[vehicleIndex-1][i] = callIndex;
        } else {
            buffer.push_back(this->representation[vehicleIndex-1][i]);
            this->representation[vehicleIndex-1][i] = buffer.front();
            buffer.pop_front();
        }

        // Update any callDetails indices
        if (pickedCalls.find(this->representation[vehicleIndex-1][i]) == pickedCalls.end()) {
            this->callDetails[this->representation[vehicleIndex-1][i]-1].indices.first = i;
            pickedCalls.insert(this->representation[vehicleIndex-1][i]);
        } else {
            this->callDetails[this->representation[vehicleIndex-1][i]-1].indices.second = i;
        }
    }

    // Update callDetails for inserted call
    this->callDetails[callIndex-1].vehicle = vehicleIndex;

    // Update cost
    this->updateCost(vehicleIndex, callIndex, indices, true, this);
}

void Solution::remove(int vehicleIndex, int callIndex) {
    // Update cost
    std::pair<int, int> callIndices = this->callDetails[callIndex-1].indices;
    this->updateCost(vehicleIndex, callIndex, callIndices, false, this);

    // Remove call from representation
    int skip = 0;
    std::unordered_set<int> pickedCalls;
    for (int i = 0; i+skip < this->representation[vehicleIndex-1].size(); i++) {
        while (i+skip < this->representation[vehicleIndex-1].size() && this->representation[vehicleIndex-1][i+skip] == callIndex) {
            skip++;
        }

        if (i+skip < this->representation[vehicleIndex-1].size()) {
            this->representation[vehicleIndex-1][i] = this->representation[vehicleIndex-1][i+skip];
        }

        // Update any callDetails indices
        if (pickedCalls.find(this->representation[vehicleIndex-1][i]) == pickedCalls.end()) {
            this->callDetails[this->representation[vehicleIndex-1][i]-1].indices.first = i;
            pickedCalls.insert(this->representation[vehicleIndex-1][i]);
        } else {
            this->callDetails[this->representation[vehicleIndex-1][i]-1].indices.second = i;
        }
    }

    // Resize the vector down
    this->representation[vehicleIndex-1].resize(this->representation[vehicleIndex-1].size()-2);
}

void Solution::move(int vehicleIndex, int callIndex, std::pair<int, int> indices) {
    // Find vehicle which currently has call
    int vehicleCall = this->callDetails[callIndex-1].vehicle;

    // If call is at the exact position already, do nothing
    if (vehicleIndex == vehicleCall && indices == this->callDetails[callIndex-1].indices) {
        return;
    }

    // If not, remove call from where it currently is and add to wanted position
    this->remove(vehicleCall, callIndex);
    this->add(vehicleIndex, callIndex, indices);
}

std::pair<int, int> Solution::outsource(int callIndex) {
    // First find insertion position
    int insertion = std::distance(this->representation[this->outsourceVehicle-1].begin(), std::lower_bound(this->representation[this->outsourceVehicle-1].begin(), this->representation[this->outsourceVehicle-1].end(), callIndex));
    std::pair<int, int> indices = std::make_pair(insertion, insertion+1);

    // Then move call to those positions
    this->move(this->outsourceVehicle, callIndex, indices);

    // Return insertion indices
    return indices;
}

bool Solution::isFeasible() {
    // Check if value is cached
    if (this->feasibilityCache.first) {
        return this->feasibilityCache.second;
    }

    // Handle our vehicles
    for (int vehicleIndex = 1; vehicleIndex <= this->problem->noVehicles; vehicleIndex++) {
        Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

        std::unordered_set<int> startedCalls;
        std::unordered_set<int> possibleCalls(vehicle.possibleCalls.begin(), vehicle.possibleCalls.end());

        int currentTime = vehicle.startTime;
        int currentCapacity = vehicle.capacity;
        int currentNode = vehicle.homeNode;

        for (int callIndex : this->representation[vehicleIndex-1]) {
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
        }

        // Verify that all picked up calls were delivered (Only validity check as it is efficient to compute)
        assert(currentCapacity == vehicle.capacity);
    }

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

    Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

    std::unordered_set<int> startedCalls;
    std::unordered_set<int> possibleCalls(vehicle.possibleCalls.begin(), vehicle.possibleCalls.end());

    int currentTime = vehicle.startTime;
    int currentCapacity = vehicle.capacity;
    int currentNode = vehicle.homeNode;

    for (int callIndex : this->representation[vehicleIndex-1]) {
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
    }

    // Verify that all picked up calls were delivered (Only validity check as it is efficient to compute)
    assert(currentCapacity == vehicle.capacity);

    // The solution is feasible!
    this->feasibilityCache = std::make_pair(true, true);
}

int Solution::getCost() {
    // Check if value is cached
    if (this->costCache.first) {
        return this->costCache.second;
    }

    // Handle our vehicles
    int totalCost = 0;
    for (int vehicleIndex = 1; vehicleIndex <= this->problem->noVehicles; vehicleIndex++) {
        // Reset cost before computing
        this->costs[vehicleIndex-1] = 0;

        Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

        int currentNode = vehicle.homeNode;
        std::unordered_set<int> startedCalls;

        for (int callIndex : this->representation[vehicleIndex-1]) {
            if (startedCalls.find(callIndex) == startedCalls.end()) {
                // Pickup call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call origin node
                this->costs[vehicleIndex-1] += vehicle.routeTimeCost[currentNode-1][call.originNode-1].cost;
                currentNode = call.originNode;

                // Pickup cargo at origin node (wait some time)
                this->costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].first.cost;

                startedCalls.insert(callIndex);
            } else {
                // Deliver call cargo
                Call call = this->problem->calls[callIndex-1];

                // Travel to call destination node
                this->costs[vehicleIndex-1] += vehicle.routeTimeCost[currentNode-1][call.destinationNode-1].cost;
                currentNode = call.destinationNode;

                // Deliver cargo at destination node (wait some time)
                this->costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].second.cost;
            }
        }
        totalCost += this->costs[vehicleIndex-1];
    }


    // Handle outsourced calls
    std::unordered_set<int> outsourcedCalls;
    this->costs[this->outsourceVehicle-1] = 0;
    for (int callIndex : this->representation[this->outsourceVehicle-1]) {

        // Only count outsourced calls once (for effiency)
        if (outsourcedCalls.find(callIndex) == outsourcedCalls.end()) {
            // Outsource the call
            Call call = this->problem->calls[callIndex-1];
            this->costs[this->outsourceVehicle-1] += call.costOfNotTransporting;

            outsourcedCalls.insert(callIndex);
        }
    }
    totalCost += this->costs[this->outsourceVehicle-1];

    //Debugger::printSolution(this);
    //std::cout << "Cost: " << std::to_string(totalCost) << "\n";

    // Cache and return the computed cost
    this->costCache = std::make_pair(true, totalCost);
    return this->costCache.second;
}

void Solution::updateCost(int vehicleIndex, int callIndex, std::pair<int, int> indices, bool insertion, Solution* previous) {
    // First retrieve the correct representation to look at
    std::vector<int> representation = insertion ? this->representation[vehicleIndex-1] : previous->representation[vehicleIndex-1];

    // Extract indices
    int index1 = indices.first, index2 = indices.second;
    
    // Operations are negated if removal compared to insertion
    int operationMultiplier = insertion ? 1 : -1;

    // Store previous cost temporarily
    int previousCost = this->costs[vehicleIndex-1];

    // Get information about the call
    Call call = this->problem->calls[callIndex-1];

    // If outsource, calculation is simple
    if (vehicleIndex == this->outsourceVehicle) {
        // Add/remove cost depending on operation
        this->costs[vehicleIndex-1] += call.costOfNotTransporting * operationMultiplier;

        // Update and cache the new cost
        int newCost = this->costCache.second - previousCost + this->costs[vehicleIndex-1];
        this->costCache = std::make_pair(true, newCost);
        return;
    }

    // Now we also need the vehicle
    Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

    // If not outsourcing, handle pickup/delivery costs here
    this->costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].first.cost * operationMultiplier;
    this->costs[vehicleIndex-1] += vehicle.callTimeCost[callIndex-1].second.cost * operationMultiplier;

    // Ensure that index1 < index2
    if (index1 > index2) {
        int temp = index2;
        index2 = index1;
        index1 = temp;
    }

    // If indices are close, cost calculation is way easier
    if (index1 == index2-1) {
        int startCallIndex = (index1 == 0) ? 0 : representation[index1-1];
        int endCallIndex = (index2 < representation.size()-1) ? representation[index2+1] : 0;
        int startNode = vehicle.homeNode;
        if (startCallIndex != 0) {
            Call startCall = this->problem->calls[startCallIndex-1];
            startNode = ((insertion ? this->callDetails[startCallIndex-1].indices.first : previous->callDetails[startCallIndex-1].indices.first) == index1-1 ? startCall.originNode : startCall.destinationNode);
        }

        // If call was added to the back, adding/removing the call is very easy
        if (endCallIndex == 0) {
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[startNode-1][call.originNode-1].cost * operationMultiplier;
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.originNode-1][call.destinationNode-1].cost * operationMultiplier;

        } else {
            Call endCall = this->problem->calls[endCallIndex-1];
            int endNode = ((insertion ? this->callDetails[endCallIndex-1].indices.first : previous->callDetails[endCallIndex-1].indices.first) == index2+1 ? endCall.originNode : endCall.destinationNode);

            // Remove old travel costs
            this->costs[vehicleIndex-1] -= vehicle.routeTimeCost[startNode-1][endNode-1].cost * operationMultiplier;

            // And add the new travel costs
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[startNode-1][call.originNode-1].cost * operationMultiplier;
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.originNode-1][call.destinationNode-1].cost * operationMultiplier;
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.destinationNode-1][endNode-1].cost * operationMultiplier;
        }
    } else {
        // First handle index1
        int startCallIndex1 = (index1 == 0) ? 0 : (insertion ? representation[index1-1] : representation[index1-1]);
        int endCallIndex1 = representation[index1+1];
        int startNode1 = vehicle.homeNode, endNode1;
        if (startCallIndex1 != 0) {
            Call startCall1 = this->problem->calls[startCallIndex1-1];
            startNode1 = ((insertion ? this->callDetails[startCallIndex1-1].indices.first : previous->callDetails[startCallIndex1-1].indices.first) == index1-1 ? startCall1.originNode : startCall1.destinationNode);
        }
        Call endCall1 = this->problem->calls[endCallIndex1-1];
        endNode1 = ((insertion ? this->callDetails[endCallIndex1-1].indices.first : previous->callDetails[endCallIndex1-1].indices.first) == index1+1 ? endCall1.originNode : endCall1.destinationNode);

        // Then index2
        int startCallIndex2 = representation[index2-1];
        int endCallIndex2 = (index2 < representation.size()-1) ? representation[index2+1] : 0;
        Call startCall2 = this->problem->calls[startCallIndex2-1];
        int startNode2 = ((insertion ? this->callDetails[startCallIndex2-1].indices.first : previous->callDetails[startCallIndex2-1].indices.first) == index2-1 ? startCall2.originNode : startCall2.destinationNode);
        int endNode2 = startNode2;
        if (endCallIndex2 != 0) {
            Call endCall2 = this->problem->calls[endCallIndex2-1];
            endNode2 = ((insertion ? this->callDetails[endCallIndex2-1].indices.first : previous->callDetails[endCallIndex2-1].indices.first) == index2+1 ? endCall2.originNode : endCall2.destinationNode);
        }

        // Remove old travel costs
        this->costs[vehicleIndex-1] -= vehicle.routeTimeCost[startNode1-1][endNode1-1].cost * operationMultiplier;
        this->costs[vehicleIndex-1] -= vehicle.routeTimeCost[startNode2-1][endNode2-1].cost * operationMultiplier;

        // And add the new travel costs
        this->costs[vehicleIndex-1] += vehicle.routeTimeCost[startNode1-1][call.originNode-1].cost * operationMultiplier; 
        this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.originNode-1][endNode1-1].cost * operationMultiplier; 
        this->costs[vehicleIndex-1] += vehicle.routeTimeCost[startNode2-1][call.destinationNode-1].cost * operationMultiplier; 
        if (endCallIndex2 != 0) {
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.destinationNode-1][endNode2-1].cost * operationMultiplier; 
        }
    }


    // Update and cache the new cost
    int newCost = this->costCache.second - previousCost + this->costs[vehicleIndex-1];
    this->costCache = std::make_pair(true, newCost);
}

std::pair<std::vector<int>, std::vector<int>> Solution::getDetails(int vehicleIndex) {
    std::vector<int> times, capacities;

    Vehicle vehicle = this->problem->vehicles[vehicleIndex-1];

    std::unordered_set<int> startedCalls;

    int currentTime = vehicle.startTime;
    int currentCapacity = vehicle.capacity;
    int currentNode = vehicle.homeNode;

    times.push_back(currentTime);
    capacities.push_back(currentCapacity);

    for (int callIndex : this->representation[vehicleIndex-1]) {
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
    }

    // Extend with at least two
    //times.push_back(currentTime);
    //times.push_back(currentTime);
    //capacities.push_back(currentCapacity);
    //capacities.push_back(currentCapacity);


    // Return the details
    return std::make_pair(times, capacities);
}


void Solution::invalidateCache() {
    this->feasibilityCache.first = false;
    this->costCache.first = false;
}