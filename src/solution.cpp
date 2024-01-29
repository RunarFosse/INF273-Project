#include "solution.h"

Solution::Solution(Problem* problemPointer) {
    // Link problem to solution
    problem = problemPointer;

    // Reserve representation size
    representation.reserve(problem->noVehicles + 2 * problem->noCalls);
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

bool Solution::isFeasible() {
    // Check if value is cached
    if (feasibilityCache.first) {
        return feasibilityCache.second;
    }

    int i = 0;
    int startedCalls = 0, finishedCalls = 0;
    std::vector<int> processedCalls(problem->noCalls);

    // Handle our vehicles
    for (int vehicleIndex = 1; vehicleIndex <= problem->noVehicles; vehicleIndex++) {
        Vehicle vehicle = problem->vehicles[vehicleIndex-1];

        std::unordered_set<int> possibleCalls(vehicle.possibleCalls.begin(), vehicle.possibleCalls.end());

        int currentTime = vehicle.startTime;
        int currentCapacity = vehicle.capacity;
        int currentNode = vehicle.homeNode;

        while (representation[i] != 0) {
            int callIndex = representation[i];

            if (possibleCalls.find(callIndex) == possibleCalls.end()) {
                // Vehicle incompatible with call
                feasibilityCache = std::make_pair(true, false);
                return feasibilityCache.second;
            }

            if (processedCalls[callIndex-1] == 0) {
                processedCalls[callIndex-1]++;
                // Pickup call cargo
                Call call = problem->calls[callIndex-1];
                
                // Travel to call origin node
                currentTime += vehicle.routeTimeCost[currentNode-1][call.originNode-1].first;
                currentNode = call.originNode;

                // Verify within time window for pickup (inclusive)
                if (currentTime < call.pickupWindow.first) {
                    // Wait if arrived early
                    currentTime = call.pickupWindow.first;
                }
                if (currentTime > call.pickupWindow.second) {
                    // Arrived outside timewindow
                    feasibilityCache = std::make_pair(true, false);
                    return feasibilityCache.second;
                }

                // Pickup cargo at origin node (wait some time)
                currentTime += vehicle.callTimeCost[callIndex-1].first.first;
                currentCapacity -= call.size;
                startedCalls++;

                // Verify capacity is not exceeded
                if (currentCapacity < 0) {
                    // Capacity exceeded
                    feasibilityCache = std::make_pair(true, false);
                    return feasibilityCache.second;
                }
                
            } else if (processedCalls[callIndex-1]== 1) {
                processedCalls[callIndex-1]++;
                // Deliver call cargo
                Call call = problem->calls[callIndex-1];

                // Travel to call origin node
                currentTime += vehicle.routeTimeCost[currentNode-1][call.destinationNode-1].first;
                currentNode = call.destinationNode;

                // Verify within time window for delivery (inclusive)
                if (currentTime < call.deliveryWindow.first) {
                    // Wait if arrived early
                    currentTime = call.deliveryWindow.first;
                }
                if (currentTime > call.deliveryWindow.second) {
                    // Arrived outside timewindow
                    feasibilityCache = std::make_pair(true, false);
                    return feasibilityCache.second;
                }

                // Deliver cargo at destination node (wait some time)
                currentTime += vehicle.callTimeCost[callIndex-1].second.first;
                currentCapacity += call.size;
                finishedCalls++;
                
            } else {
                // Call is processed more than twice, error
                feasibilityCache = std::make_pair(true, false);
                return feasibilityCache.second;
            }
            i++;
        }

        // Verify that all picked up calls were delivered
        if (startedCalls != finishedCalls) {
            // Did not finish all started calls
            feasibilityCache = std::make_pair(true, false);
            return feasibilityCache.second;
        }

        i++;
    }

    // Handle outsourced calls
    while (i < representation.size()) {
        int callIndex = representation[i];

        if (processedCalls[callIndex-1] == 0) {
            processedCalls[callIndex-1]++;
            startedCalls++;
        } else if (processedCalls[callIndex-1] == 1) {
            processedCalls[callIndex-1]++;
            finishedCalls++;
        } else {
            // Call is processed more than twice, error (even if it is outsourced)
            feasibilityCache = std::make_pair(true, false);
            return feasibilityCache.second;
        }

        i++;
    }

    // Verify that all calls were handled
    if (startedCalls != finishedCalls || finishedCalls != problem->noCalls) {
        // Not all calls were handled
        feasibilityCache = std::make_pair(true, false);
        return feasibilityCache.second;
    }

    // The solution is feasible!
    feasibilityCache = std::make_pair(true, true);
    return feasibilityCache.second;
}