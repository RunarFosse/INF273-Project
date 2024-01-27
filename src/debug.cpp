#include "debug.h"

void Debugger::printProblem(Problem* problem) {

    std::cout << "Number of nodes: " << std::to_string(problem->noNodes) << std::endl;
    std::cout << "Number of vehicles: " << std::to_string(problem->noVehicles) << std::endl;
    std::cout << "Number of calls: " << std::to_string(problem->noCalls) << std::endl;

    std::cout << "For each vehicle: vehicle index, home node, starting time, capacity" << std::endl;
    for (int i = 0; i < problem->noVehicles; i++) {
        std::cout << std::to_string(problem->vehicles[i].index) << ", " << std::to_string(problem->vehicles[i].homeNode) << ", " << std::to_string(problem->vehicles[i].startTime) << ", " << std::to_string(problem->vehicles[i].capacity) << std::endl;
    }
};