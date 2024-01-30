#include "parser.h"

Problem Parser::parseProblem(std::string path) {
    // Create a problem instance
    Problem problem = Problem();

    // Read from the given data file
    std::string line;
    std::ifstream file(path);
    if (file.is_open()) {

        // Parse the number of nodes
        file.ignore(LONG_MAX, '\n');
        std::getline(file, line);
        problem.noNodes = std::stoi(line);

        // Parse the number of vehicles
        file.ignore(LONG_MAX, '\n');
        std::getline(file, line);
        problem.noVehicles = std::stoi(line);

        // Reserve space for vehicles
        problem.vehicles.resize(problem.noVehicles);

        // Parse information per vehicle
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noVehicles; i++) {
            std::getline(file, line);
            int pointer1 = 0, pointer2 = line.find(',');

            int index = std::stoi(line.substr(pointer1, pointer2));

            Vehicle* vehicle = &problem.vehicles[index-1];
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            vehicle->homeNode = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            vehicle->startTime = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1;
            vehicle->capacity = std::stoi(line.substr(pointer1));

            // Reserve space for the vehicle edge time/cost
            vehicle->routeTimeCost.resize(problem.noNodes);
            for (int j = 0; j < problem.noNodes; j++) {
                vehicle->routeTimeCost[j].resize(problem.noNodes);
            }
        }

        // Parse the number of calls
        file.ignore(LONG_MAX, '\n');
        std::getline(file, line);
        problem.noCalls = std::stoi(line);

        // Reserve space for vehicles
        problem.calls.resize(problem.noCalls);

        // Parse possible calls per vehicle
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noVehicles; i++) {
            std::getline(file, line);
            int pointer1 = 0, pointer2 = line.find(',');

            // If the current line has no other entries other than vehicle index, skip this iteration
            if (pointer2 == std::string::npos || pointer2 == line.length()-2) {
                continue;
            }
            
            int vehicleIndex = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            while (pointer2 != std::string::npos) {
                problem.vehicles[vehicleIndex-1].possibleCalls.push_back(std::stoi(line.substr(pointer1, pointer2)));
                pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            }
            problem.vehicles[vehicleIndex-1].possibleCalls.push_back(std::stoi(line.substr(pointer1)));

            // Reserve space for the call node time/cost
            problem.vehicles[vehicleIndex-1].callTimeCost.resize(problem.noCalls);
        }

        // Parse information per call
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noCalls; i++) {
            std::getline(file, line);
            int pointer1 = 0, pointer2 = line.find(',');

            int index = std::stoi(line.substr(pointer1, pointer2));

            Call* call = &problem.calls[index-1];
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call->originNode = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call->destinationNode= std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call->size= std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call->costOfNotTransporting = std::stoi(line.substr(pointer1));
            
            int lowerbound, upperbound;
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            lowerbound = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            upperbound = std::stoi(line.substr(pointer1, pointer2));
            call->pickupWindow = {lowerbound, upperbound};

            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            lowerbound = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1;
            upperbound = std::stoi(line.substr(pointer1));
            call->deliveryWindow = {lowerbound, upperbound};
        }

        // Parse time/cost per edge per vehicle
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noNodes; i++) {
            for (int j = 0; j < problem.noNodes; j++) {
                for (int k = 0; k < problem.noVehicles; k++) {
                    std::getline(file, line);
                    int pointer1 = 0, pointer2 = line.find(',');

                    int vehicleIndex = std::stoi(line.substr(pointer1, pointer2));
                    pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
                    int originNode = std::stoi(line.substr(pointer1, pointer2));
                    pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
                    int destinationNode = std::stoi(line.substr(pointer1, pointer2));
                    pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
                    int travelTime = std::stoi(line.substr(pointer1, pointer2));
                    pointer1 = pointer2+1;
                    int travelCost = std::stoi(line.substr(pointer1));

                    problem.vehicles[vehicleIndex-1].routeTimeCost[originNode-1][destinationNode-1] = {travelTime, travelCost};
                }
            }
        }

        // Parse time/cost for each call per vehicle
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noVehicles; i++) {
            for (int j = 0; j < problem.noCalls; j++) {
                std::getline(file, line);
                int pointer1 = 0, pointer2 = line.find(',');

                int vehicleIndex = std::stoi(line.substr(pointer1, pointer2));
                pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
                int callIndex = std::stoi(line.substr(pointer1, pointer2));
                pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);

                int originNodeTime = std::stoi(line.substr(pointer1, pointer2));
                pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
                int originNodeCost = std::stoi(line.substr(pointer1, pointer2));
                pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
                int destinationNodeTime = std::stoi(line.substr(pointer1, pointer2));
                pointer1 = pointer2+1;
                int destinationNodeCost = std::stoi(line.substr(pointer1));
                
                TimeCost originTimeCost = {originNodeTime, originNodeCost};
                TimeCost destinationTimeCost = {destinationNodeTime, destinationNodeCost};
                problem.vehicles[vehicleIndex-1].callTimeCost[callIndex-1] = std::make_pair(originTimeCost, destinationTimeCost);
            }
        }

        // Close the file after finished reading
        file.close();
    } else {
        // If the file is not open, something went wrong
        std::cerr << "ERROR: Couldn't open data file '" << path << "'" << std::endl;
    }

    // Return the problem instance
    return problem;
}