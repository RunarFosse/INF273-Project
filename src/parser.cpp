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

            // Add a vector containing all vehicles which can take this call
            for (int vehicleIndex = 1; vehicleIndex <= problem.noVehicles; vehicleIndex++) {
                for (int possibleCall : problem.vehicles[vehicleIndex-1].possibleCalls) {
                    if (possibleCall == index) {
                        call->possibleVehicles.push_back(vehicleIndex);
                        break;
                    }
                }
            }
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

    // Lambda to easily average distance between calls over all vehicles
    auto calculateMeanDistance = [problem](int node1, int node2) {
        double distance = 0;
        for (int vehicleIndex = 1; vehicleIndex <= problem.noVehicles; vehicleIndex++) {
            distance += problem.vehicles[vehicleIndex-1].routeTimeCost[node1-1][node2-1].time / (double)problem.noVehicles;
        }
        return distance;
    };

    // At last, calculate similarity per call
    double phi = 3.0, chi = 1.5, psi = 0.1, omega = 1000.0;
    for (int callIndex = 1; callIndex <= problem.noCalls; callIndex++) {
        Call& call = problem.calls[callIndex-1];
        for (int otherCallIndex = 1; otherCallIndex <= problem.noCalls; otherCallIndex++) {
            if (otherCallIndex == callIndex) {
                continue;
            }
            
            Call& otherCall = problem.calls[otherCallIndex-1];

            // Calculate shared vehicles between both calls
            std::vector<int> sharedVehicles;
            std::set_intersection(call.possibleVehicles.begin(), call.possibleVehicles.end(), otherCall.possibleVehicles.begin(), otherCall.possibleVehicles.end(), std::back_inserter(sharedVehicles));

            // Formulate the similarity
            Similarity similarity;
            similarity.callIndex = otherCallIndex;
            similarity.relatedness = phi * (calculateMeanDistance(call.originNode, otherCall.originNode) + calculateMeanDistance(call.destinationNode, otherCall.destinationNode))
            + chi * (std::abs(call.pickupWindow.end - otherCall.pickupWindow.end) + std::abs(call.deliveryWindow.end - otherCall.deliveryWindow.end))
            + psi * std::abs(call.size  - otherCall.size)
            + omega * (1 - sharedVehicles.size() / std::min(call.possibleVehicles.size(), otherCall.possibleVehicles.size()));

            problem.calls[callIndex-1].similarities.push_back(similarity);
        }

        // Sort similarities from most- to least-similar
        std::sort(problem.calls[callIndex-1].similarities.begin(), problem.calls[callIndex-1].similarities.end(), [](const Similarity& a, const Similarity& b) {
            return a.relatedness < b.relatedness;
        });
    }

    // Return the problem instance
    return problem;
}