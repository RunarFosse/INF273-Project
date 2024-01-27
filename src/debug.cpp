#include "debug.h"

void Debugger::printProblem(Problem* problem) {

    std::cout << "Number of nodes: " << std::to_string(problem->noNodes) << std::endl;
    std::cout << "Number of vehicles: " << std::to_string(problem->noVehicles) << std::endl;
    std::cout << "Number of calls: " << std::to_string(problem->noCalls) << std::endl;

    std::cout << "For each vehicle: vehicle index, home node, starting time, capacity" << std::endl;
    for (int i = 0; i < problem->noVehicles; i++) {
        std::cout << std::to_string(problem->vehicles[i].index) << ", ";
        std::cout << std::to_string(problem->vehicles[i].homeNode) << ", ";
        std::cout << std::to_string(problem->vehicles[i].startTime) << ", ";
        std::cout << std::to_string(problem->vehicles[i].capacity) << std::endl;
    }

    std::cout << "For each vehicle: vehicle index, and then a list of calls that can be transported using that vehicle" << std::endl;
    for (int i = 0; i < problem->noVehicles; i++) {
        std::cout << std::to_string(problem->vehicles[i].index) << ", [ ";
        for (int possibleCall : problem->vehicles[i].possibleCalls) {
            std::cout << std::to_string(possibleCall) << " ";
        }
        std::cout << "]" << std::endl;
    }

    std::cout << "For each call: call index, origin node, destination node, size, cost of not transporting, timewindow for pickup, timewindow for delivery" << std::endl;
    for (int i = 0; i < problem->noCalls; i++) {
        std::cout << std::to_string(problem->calls[i].index) << ", ";
        std::cout << std::to_string(problem->calls[i].originNode) << ", ";
        std::cout << std::to_string(problem->calls[i].destinationNode) << ", ";
        std::cout << std::to_string(problem->calls[i].size) << ", ";
        std::cout << std::to_string(problem->calls[i].costOfNotTransporting) << ", ";
        std::cout << "(" << std::to_string(problem->calls[i].pickupWindow.first) << ", ";
        std::cout << std::to_string(problem->calls[i].pickupWindow.second) << "), ";
        std::cout << "(" << std::to_string(problem->calls[i].deliveryWindow.first) << ", ";
        std::cout << std::to_string(problem->calls[i].deliveryWindow.second) << ")" << std::endl;
    }
};