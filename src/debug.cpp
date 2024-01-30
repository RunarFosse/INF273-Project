#include "debug.h"

void Debugger::printProblem(Problem* problem, bool printMap) {
    std::cout << "Number of nodes: " << std::to_string(problem->noNodes) << std::endl;
    std::cout << "Number of vehicles: " << std::to_string(problem->noVehicles) << std::endl;
    std::cout << "Number of calls: " << std::to_string(problem->noCalls) << std::endl;

    std::cout << "For each vehicle: vehicle index, home node, starting time, capacity" << std::endl;
    for (int i = 1; i <= problem->noVehicles; i++) {
        std::cout << std::to_string(i) << ", ";
        std::cout << std::to_string(problem->vehicles[i-1].homeNode) << ", ";
        std::cout << std::to_string(problem->vehicles[i-1].startTime) << ", ";
        std::cout << std::to_string(problem->vehicles[i-1].capacity) << std::endl;
    }

    std::cout << "For each vehicle: vehicle index, and then a list of calls that can be transported using that vehicle" << std::endl;
    for (int i = 1; i <= problem->noVehicles; i++) {
        std::cout << std::to_string(i) << ", [ ";
        for (int possibleCall : problem->vehicles[i-1].possibleCalls) {
            std::cout << std::to_string(possibleCall) << " ";
        }
        std::cout << "]" << std::endl;
    }

    std::cout << "For each call: call index, origin node, destination node, size, cost of not transporting, timewindow for pickup, timewindow for delivery" << std::endl;
    for (int i = 1; i <= problem->noCalls; i++) {
        std::cout << std::to_string(i) << ", ";
        std::cout << std::to_string(problem->calls[i-1].originNode) << ", ";
        std::cout << std::to_string(problem->calls[i-1].destinationNode) << ", ";
        std::cout << std::to_string(problem->calls[i-1].size) << ", ";
        std::cout << std::to_string(problem->calls[i-1].costOfNotTransporting) << ", ";
        std::cout << "(" << std::to_string(problem->calls[i-1].pickupWindow.first) << ", ";
        std::cout << std::to_string(problem->calls[i-1].pickupWindow.second) << "), ";
        std::cout << "(" << std::to_string(problem->calls[i-1].deliveryWindow.first) << ", ";
        std::cout << std::to_string(problem->calls[i-1].deliveryWindow.second) << ")" << std::endl;
    }

    if (printMap) {
        std::cout << "For each vehicle's node edge: vehicle index, origin node, destination node, travel time (in hours), travel cost (in Euro)" << std::endl;
        for (int i = 1; i <= problem->noNodes; i++) {
            for (int j = 1; j <= problem->noNodes; j++) {
                for (int k = 1; k <= problem->noVehicles; k++) {
                    std::cout << std::to_string(k) << ", ";
                    std::cout << std::to_string(i) << ", ";
                    std::cout << std::to_string(j) << ", ";
                    std::cout << std::to_string(problem->vehicles[k-1].routeTimeCost[i-1][j-1].first) << ", ";
                    std::cout << std::to_string(problem->vehicles[k-1].routeTimeCost[i-1][j-1].second) << std::endl;
                }
            }
        }
    }

    std::cout << "For each vehicle's call: vehicle, call, origin node time (in hours), origin node costs (in Euro), destination node time (in hours), destination node costs (in Euro)" << std::endl;
    for (int i = 1; i <= problem->noVehicles; i++) {
        for (int j = 1; j <= problem->noCalls; j++) {
            std::cout << std::to_string(i) << ", ";
            std::cout << std::to_string(j) << ", ";
            std::cout << "(" << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].first.first) << ", ";
            std::cout << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].first.second) << "), ";
            std::cout << "(" << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].second.first) << ", ";
            std::cout << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].second.second) << ")" << std::endl;
        }
    }
};

void Debugger::printSolution(Solution* solution) {
    std::cout << "[";

    for (int i = 0; i < solution->representation.size(); i++) {
        std::cout << std::to_string(solution->representation[i]);
        if (i < solution->representation.size()-1) {
            std::cout << ", ";
        }
    }

    std::cout << "]" << std::endl;
};