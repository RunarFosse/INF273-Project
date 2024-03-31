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

    // Reserve representation size
    this->representation.resize(problem->noVehicles+1);
    this->costs.resize(problem->noVehicles+1);

    this->callDetails.resize(problem->noCalls);

    // Infer representation from what is given
    int currentVehicle = 1, lastSeperator = -1;
    std::unordered_set<int> pickedCalls;
    for (int i = 0; i < representation.size(); i++) {
        int callIndex = representation[i];
        if (callIndex == 0) {
            lastSeperator = i;
            currentVehicle++;
            continue;
        }

        if (pickedCalls.find(callIndex) == pickedCalls.end()) {
            this->callDetails[callIndex-1].vehicle = currentVehicle;
            this->callDetails[callIndex-1].indices.first = i - lastSeperator+1;
            pickedCalls.insert(callIndex);
        } else {
            this->callDetails[callIndex-1].indices.second = i - lastSeperator+1;
        }

        this->representation[currentVehicle-1].push_back(callIndex);
    }

    // Precompute feasibility
    this->isFeasible();

    // And then precalcuate cost
    this->getCost();
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
    std::vector<int>& representation = this->representation[vehicleIndex-1];

    // Resize the vector up
    representation.resize(representation.size()+2);

    // Denote start of iteration
    int start = indices.first;
    
    // Add call to representation
    std::deque<int> buffer;
    std::unordered_set<int> pickedCalls;
    for (int i = start; i < representation.size(); i++) {
        if (i == indices.first || i == indices.second) {
            buffer.push_back(representation[i]);
            representation[i] = callIndex;
        } else {
            buffer.push_back(representation[i]);
            representation[i] = buffer.front();
            buffer.pop_front();
        }

        // Update any callDetails indices
        if (this->callDetails[representation[i]-1].indices.first >= start and pickedCalls.find(representation[i]) == pickedCalls.end()) {
            this->callDetails[representation[i]-1].indices.first = i;
            pickedCalls.insert(representation[i]);
        } else {
            this->callDetails[representation[i]-1].indices.second = i;
        }
    }

    // Update callDetails for inserted call
    this->callDetails[callIndex-1] = {vehicleIndex, indices, false};

    // And then update the cost
    this->updateCost(callIndex, true);
}

void Solution::remove(int callIndex) {
    int vehicleIndex = this->callDetails[callIndex-1].vehicle;
    std::vector<int>& representation = this->representation[vehicleIndex-1];

    // Denote start of iteration
    int start = this->callDetails[callIndex-1].indices.first;

    // Remove call from representation
    int skip = 0;
    std::unordered_set<int> pickedCalls;
    for (int i = start; i+skip < representation.size(); i++) {
        while (i+skip < representation.size() && representation[i+skip] == callIndex) {
            skip++;
        }

        if (i+skip < representation.size()) {
            representation[i] = representation[i+skip];
            
            // Update any callDetails indices
            if (this->callDetails[representation[i]-1].indices.first >= start and pickedCalls.find(representation[i]) == pickedCalls.end()) {
                this->callDetails[representation[i]-1].indices.first = i;
                pickedCalls.insert(representation[i]);
            } else {
                this->callDetails[representation[i]-1].indices.second = i;
            }
        }
    }

    // Set callDetail to removed
    this->callDetails[callIndex-1].removed = true;

    // Resize the vector down
    representation.resize(representation.size()-2);

    // And update the cost
    this->updateCost(callIndex, false);
}

void Solution::move(int vehicleIndex, int callIndex, std::pair<int, int> indices) {
    // Remove call from where it currently is (if not already) and add to wanted position
    if (!this->callDetails[callIndex-1].removed) {
        this->remove(callIndex);
    }
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

std::pair<int, bool> Solution::updateFeasibility(int vehicleIndex, int startIndex, int startTime, int startCapacity) {
    // Initialize feasibility information
    std::pair<int, bool> feasibilityInformation = std::make_pair(-1, false);

    // Early return as outsource is always feasible
    if (vehicleIndex == this->problem->noVehicles+1) {
        this->feasibilityCache = std::make_pair(true, true);
        return feasibilityInformation;
    }  

    Vehicle& vehicle = this->problem->vehicles[vehicleIndex-1];
    std::vector<int>& representation = this->representation[vehicleIndex-1];

    std::unordered_set<int> startedCalls;
    std::unordered_set<int> possibleCalls(vehicle.possibleCalls.begin(), vehicle.possibleCalls.end());

    int currentTime = startTime == 0 ? vehicle.startTime : startTime;
    int currentCapacity = startCapacity == 0 ? vehicle.capacity : startCapacity;
    int currentNode = vehicle.homeNode;
    if (startIndex > 0) {
        Call& startCall = this->problem->calls[representation[startIndex-1]-1];
        if (this->callDetails[representation[startIndex-1]-1].indices.first == startIndex-1) {
            currentNode = startCall.originNode;
        } else {
            currentNode = startCall.destinationNode;
        }
    }

    for (int i = startIndex; i < representation.size(); i++) {
        int callIndex = representation[i];
        if (possibleCalls.find(callIndex) == possibleCalls.end()) {
            // Vehicle incompatible with call
            this->feasibilityCache = std::make_pair(true, false);
            // This should throw an error, means something is wrong with implementation
            assert(false);
            return feasibilityInformation;
        }

        if (this->callDetails[callIndex-1].indices.first >= startIndex and startedCalls.find(callIndex) == startedCalls.end()) {
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
                feasibilityInformation.first = i;
                return feasibilityInformation;
            }

            // Pickup cargo at origin node (wait some time)
            currentTime += vehicle.callTimeCost[callIndex-1].first.time;
            currentCapacity -= call.size;

            // Verify capacity is not exceeded
            if (currentCapacity < 0) {
                // Capacity exceeded
                this->feasibilityCache = std::make_pair(true, false);
                feasibilityInformation.first = i;
                feasibilityInformation.second = true;
                return feasibilityInformation;
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
                feasibilityInformation.first = i;
                return feasibilityInformation;
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
    return feasibilityInformation;
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

    // Cache and return the computed cost
    this->costCache = std::make_pair(true, totalCost);
    return this->costCache.second;
}

void Solution::updateCost(int callIndex, bool insertion) {
    // If cost has not been computed, fully compute
    if (!this->costCache.first) {
        this->getCost();
        return;
    }

    // Unpack details
    auto [vehicleIndex, indices, removed] = this->callDetails[callIndex-1];
    auto [index1, index2] = indices;

    // Keep a multiplier deciding if we should add or remove cost
    int operationMultiplier = insertion ? 1 : -1;

    // Store previous cost
    int previousCost = this->costs[vehicleIndex-1];
    int newCost = previousCost;

    // Get information about the call
    Call& call = this->problem->calls[callIndex-1];

    // If we are dealing with the outsource vehicle, computation is trivial!
    if (vehicleIndex == this->outsourceVehicle) {
        newCost += call.costOfNotTransporting * operationMultiplier;
        this->costs[vehicleIndex-1] = newCost;

        // Update cost cache to new cost and return
        this->costCache.second += newCost - previousCost;
        return;
    }

    // Get information about the vehicle
    Vehicle& vehicle = this->problem->vehicles[vehicleIndex-1];

    // Shortcut current vehicles representation
    std::vector<int>& representation = this->representation[vehicleIndex-1];

    // Declare start/end indices for each insertion/removal point
    int startIndex1 = index1-1;
    int endIndex1 = insertion ? index1+1 : index1;
    int startIndex2 = insertion ? index2-1 : index2-2;
    int endIndex2 = insertion ? index2+1 : index2-1;

    // Declare lambda to easily get node of a given call's index
    auto getNodeOf = [this](int callIndex, int index) {
        return (this->callDetails[callIndex-1].indices.first == index) ? this->problem->calls[callIndex-1].originNode : this->problem->calls[callIndex-1].destinationNode;
    };

    // If insertions/removals are next to eachother, computations are a tiny bit different
    bool close = (index1 == index2-1);


    // Then add/remove cost of picking up and delivering the selected call
    newCost += (vehicle.callTimeCost[callIndex-1].first.cost + vehicle.callTimeCost[callIndex-1].second.cost) * operationMultiplier;
   
    // If insertions/removals are close together, computation becomes easier
    if (close) {
        int startNode = (startIndex1 == -1) ? vehicle.homeNode : getNodeOf(representation[startIndex1], startIndex1);
       
        newCost += vehicle.routeTimeCost[startNode-1][call.originNode-1].cost * operationMultiplier;
        newCost += vehicle.routeTimeCost[call.originNode-1][call.destinationNode-1].cost * operationMultiplier;
        if (endIndex2 < representation.size()) {
            int endNode = getNodeOf(representation[endIndex2], endIndex2);

            newCost += vehicle.routeTimeCost[call.destinationNode-1][endNode-1].cost * operationMultiplier;
            newCost -= vehicle.routeTimeCost[startNode-1][endNode-1].cost * operationMultiplier;
        }
    } else {
        int startNode1 = (startIndex1 == -1) ? vehicle.homeNode : getNodeOf(representation[startIndex1], startIndex1);
        int endNode1 = getNodeOf(representation[endIndex1], endIndex1);

        newCost += vehicle.routeTimeCost[startNode1-1][call.originNode-1].cost * operationMultiplier;
        newCost += vehicle.routeTimeCost[call.originNode-1][endNode1-1].cost * operationMultiplier;
        newCost -= vehicle.routeTimeCost[startNode1-1][endNode1-1].cost * operationMultiplier;

        int startNode2 = getNodeOf(representation[startIndex2], startIndex2);

        newCost += vehicle.routeTimeCost[startNode2-1][call.destinationNode-1].cost * operationMultiplier;
        if (endIndex2 < representation.size()) {
            int endNode2 = getNodeOf(representation[endIndex2], endIndex2);

            newCost += vehicle.routeTimeCost[call.destinationNode-1][endNode2-1].cost * operationMultiplier;
            newCost -= vehicle.routeTimeCost[startNode2-1][endNode2-1].cost * operationMultiplier;
        }
    }

    // Update cost cache to new cost and return
    this->costs[vehicleIndex-1] = newCost;
    this->costCache.second += newCost - previousCost;
    return;
}

std::pair<std::vector<int>, std::vector<int>> Solution::getDetails(int vehicleIndex, int timeConstraint) {
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

        if (currentTime > timeConstraint) {
            break;
        }
    }

    // Return the details
    return std::make_pair(times, capacities);
}


void Solution::invalidateCache() {
    this->feasibilityCache.first = false;
    this->costCache.first = false;
}