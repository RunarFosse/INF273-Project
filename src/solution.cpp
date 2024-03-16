#include "solution.h"

#include "debug.h"

Solution::Solution(Problem* problem) {
    // Link problem to solution
    this->problem = problem;

    // Reserve representation size
    this->representation.reserve(problem->noVehicles + 2 * problem->noCalls);
    this->seperators.reserve(problem->noVehicles+1);
    this->costs.resize(problem->noVehicles+1);

    this->callDetails.resize(problem->noCalls);
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

    // Precompute feasibility
    this->isFeasible();
}

Solution Solution::copy() {
    // Create a new solution with the same problem
    Solution solution = Solution(this->problem);

    // Copy over representation, seperator and cost vectors
    solution.representation = this->representation;
    solution.seperators = this->seperators;
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
    solution.isFeasible();

    // And then precalcuate cost
    solution.getCost();

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

    // Precalculate cost and feasibility
    solution.isFeasible();
    solution.getCost();

    // Return the randomized solution
    return solution;
}

void Solution::greedyMove(int callIndex, int from, int to, int index1, int index2) {
    // TODO: Make one-pass algorithm

    int start = this->callDetails[callIndex-1].first, end = this->callDetails[callIndex-1].second;
    if (start == index1 && end == index2) {
        return;
    }

    int skip = 0;
    for (int i = start; i+skip <= std::max(end, index2); i++) {
        while (i+skip < this->representation.size() && this->representation[i+skip] == callIndex) {
            skip++;
        }
        if (i+skip < this->representation.size()) {
            this->representation[i] = this->representation[i+skip];
        }
    }

    std::unordered_set<int> pickedCalls;

    std::deque<int> buffer;
    int currentVehicle = std::distance(this->seperators.begin(), std::upper_bound(this->seperators.begin(), this->seperators.end(), from));
    for (int i = from; i <= std::max(end, index2); i++) {
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
        } else {
            if (pickedCalls.find(this->representation[i]) == pickedCalls.end()) {
                this->callDetails[this->representation[i]-1].first = i;
                pickedCalls.insert(this->representation[i]);
            } else {
                this->callDetails[this->representation[i]-1].second = i;
            }
        }
    }
}

void Solution::move(int callIndex, int index1, int index2) {
    // TODO: Make one-pass algorithm

    int start = this->callDetails[callIndex-1].first, end = this->callDetails[callIndex-1].second;
    if (start == index1 && end == index2) {
        return;
    }

    int skip = 0;
    for (int i = start; i+skip <= std::max(end, index2); i++) {
        while (i+skip < this->representation.size() && this->representation[i+skip] == callIndex) {
            skip++;
        }
        if (i+skip < this->representation.size()) {
            this->representation[i] = this->representation[i+skip];
        }
    }

    std::unordered_set<int> pickedCalls;

    std::deque<int> buffer;
    int currentVehicle = 1;
    for (int i = 0; i <= std::max(end, index2); i++) {
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
        } else {
            if (pickedCalls.find(this->representation[i]) == pickedCalls.end()) {
                this->callDetails[this->representation[i]-1].first = i;
                pickedCalls.insert(this->representation[i]);
            } else {
                this->callDetails[this->representation[i]-1].second = i;
            }
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

void Solution::outsourceSeveral(std::vector<int> callIndices) {
    // Create a set
    std::set<int> calls(callIndices.begin(), callIndices.end());

    // Then outsource every single one in one pass
    int currentVehicle = 1, i = 0, skip = 0;
    while (currentVehicle <= this->problem->noVehicles) {
        while (calls.find(this->representation[i+skip]) != calls.end()) {
            skip++;
        }

        this->representation[i] = this->representation[i+skip];

        // If passed a 0, update seperators
        if (this->representation[i] == 0) {
            this->seperators[currentVehicle] = i;
            currentVehicle++;
        }
        i++;
    }
    
    for (int callIndex : calls) {
        while (i+skip < this->representation.size() && this->representation[i+skip] < callIndex) {
            this->representation[i] = this->representation[i+skip];
            i++;
        }
        if (i+skip < this->representation.size() && this->representation[i+skip] > callIndex) {
            skip -= 2;
        }
        this->representation[i] = callIndex;
        this->representation[i+1] = callIndex;
        this->callDetails[callIndex-1] = std::make_pair(i, i+1);
        i += 2;
    }
}

int Solution::getVehicleWith(int callIndex) {
    // Binary search vehicle containing position of callIndex
    int firstOccurence = this->callDetails[callIndex-1].first;

    // Easy check if outsourced
    if (firstOccurence > this->seperators[this->problem->noVehicles]) {
        return this->problem->noVehicles+1;
    }

    int left = 0, right = this->problem->noVehicles;
    while (left <= right) {
        int pivot = (left+right)/2;

        if (firstOccurence > this->seperators[pivot] && firstOccurence < this->seperators[pivot+1]) {
            return pivot+1;
        }
        if (left == right) {
            return left+1;
        }

        if (firstOccurence > this->seperators[pivot]) {
            left = pivot;
        } else {
            right = pivot;
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

                // Set index of pickup
                this->callDetails[callIndex-1].first = i;

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

                // Set index of delivery
                this->callDetails[callIndex-1].second = i;

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
    while (++i < this->representation.size()) {
        this->callDetails[this->representation[i]-1].first = i;
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

            // Set index of pickup
            this->callDetails[callIndex-1].first = i;

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

            // Set index of delivery
            this->callDetails[callIndex-1].second = i;

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

void Solution::updateCost(int vehicleIndex, int callIndex, int index1, int index2, bool insertion, Solution* previous) {
    // Operations are negated if removal compared to insertion
    int operationMultiplier = insertion ? 1 : -1;

    // Store previous cost temporarily
    int previousCost = this->costs[vehicleIndex-1];

    // Get information about the call
    Call call = this->problem->calls[callIndex-1];

    // If outsource, calculation is simple
    if (vehicleIndex == this->problem->noVehicles+1) {
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
        int startCallIndex = (index1 == 0) ? 0 : (insertion ? this->representation[index1-1] : previous->representation[index1-1]);
        int endCallIndex = insertion ? this->representation[index2+1] : previous->representation[index2+1];
        int startNode = vehicle.homeNode;
        if (startCallIndex != 0) {
            Call startCall = this->problem->calls[startCallIndex-1];
            startNode = ((insertion ? this->callDetails[startCallIndex-1].first : previous->callDetails[startCallIndex-1].first) == index1-1 ? startCall.originNode : startCall.destinationNode);
        }

        // If call was added to the back, adding/removing the call is very easy
        if (endCallIndex == 0) {
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[startNode-1][call.originNode-1].cost * operationMultiplier;
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.originNode-1][call.destinationNode-1].cost * operationMultiplier;

        } else {
            Call endCall = this->problem->calls[endCallIndex-1];
            int endNode = ((insertion ? this->callDetails[endCallIndex-1].first : previous->callDetails[endCallIndex-1].first) == index2+1 ? endCall.originNode : endCall.destinationNode);

            // Remove old travel costs
            this->costs[vehicleIndex-1] -= vehicle.routeTimeCost[startNode-1][endNode-1].cost * operationMultiplier;

            // And add the new travel costs
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[startNode-1][call.originNode-1].cost * operationMultiplier;
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.originNode-1][call.destinationNode-1].cost * operationMultiplier;
            this->costs[vehicleIndex-1] += vehicle.routeTimeCost[call.destinationNode-1][endNode-1].cost * operationMultiplier;
        }
    } else {
        // First handle index1
        int startCallIndex1 = (index1 == 0) ? 0 : (insertion ? this->representation[index1-1] : previous->representation[index1-1]);
        int endCallIndex1 = insertion ? this->representation[index1+1] : previous->representation[index1+1];
        int startNode1 = vehicle.homeNode, endNode1;
        if (startCallIndex1 != 0) {
            Call startCall1 = this->problem->calls[startCallIndex1-1];
            startNode1 = ((insertion ? this->callDetails[startCallIndex1-1].first : previous->callDetails[startCallIndex1-1].first) == index1-1 ? startCall1.originNode : startCall1.destinationNode);
        }
        Call endCall1 = this->problem->calls[endCallIndex1-1];
        endNode1 = ((insertion ? this->callDetails[endCallIndex1-1].first : previous->callDetails[endCallIndex1-1].first) == index1+1 ? endCall1.originNode : endCall1.destinationNode);

        // Then index2
        int startCallIndex2 = insertion ? this->representation[index2-1] : previous->representation[index2-1];
        int endCallIndex2 = insertion ? this->representation[index2+1] : previous->representation[index2+1];
        Call startCall2 = this->problem->calls[startCallIndex2-1];
        int startNode2 = ((insertion ? this->callDetails[startCallIndex2-1].first : previous->callDetails[startCallIndex2-1].first) == index2-1 ? startCall2.originNode : startCall2.destinationNode);
        int endNode2 = startNode2;
        if (endCallIndex2 != 0) {
            Call endCall2 = this->problem->calls[endCallIndex2-1];
            endNode2 = ((insertion ? this->callDetails[endCallIndex2-1].first : previous->callDetails[endCallIndex2-1].first) == index2+1 ? endCall2.originNode : endCall2.destinationNode);
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