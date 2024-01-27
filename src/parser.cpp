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

        // Parse information per vehicle
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noVehicles; i++) {
            std::getline(file, line);
            int pointer1 = 0, pointer2 = line.find(',');

            Vehicle vehicle;
            vehicle.index = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            vehicle.homeNode = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            vehicle.startTime = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1;
            vehicle.capacity = std::stoi(line.substr(pointer1));

            problem.vehicles.push_back(vehicle);
        }
        // Sort the vector based on the vehicle index (vehicle index might not match vector index)
        std::sort(problem.vehicles.begin(), problem.vehicles.end(), [ ](const Vehicle& lhs, const Vehicle& rhs) {
            return lhs.index < rhs.index;
        });


        // Parse the number of calls
        file.ignore(LONG_MAX, '\n');
        std::getline(file, line);
        problem.noCalls = std::stoi(line);

        // Parse possible calls per vehicle
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noVehicles; i++) {
            std::getline(file, line);
            int pointer1 = 0, pointer2 = line.find(',');

            // If the current line has no other entries other than vehicle index, skip this iteration
            if (pointer2 == std::string::npos || pointer2 == line.length()-2)
                continue;

            int vehicleIndex = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            while (pointer2 != std::string::npos) {
                problem.vehicles[vehicleIndex-1].possibleCalls.push_back(std::stoi(line.substr(pointer1, pointer2)));
                pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            }
            problem.vehicles[vehicleIndex-1].possibleCalls.push_back(std::stoi(line.substr(pointer1)));
        }

        // Parse information per call
        file.ignore(LONG_MAX, '\n');
        for (int i = 0; i < problem.noCalls; i++) {
            std::getline(file, line);
            int pointer1 = 0, pointer2 = line.find(',');

            Call call;
            call.index = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call.originNode = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call.destinationNode= std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call.size= std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            call.costOfNotTransporting = std::stoi(line.substr(pointer1));
            
            int lowerbound, upperbound;
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            lowerbound = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            upperbound = std::stoi(line.substr(pointer1, pointer2));
            call.pickupWindow = std::make_pair(lowerbound, upperbound);

            pointer1 = pointer2+1, pointer2 = line.find(',', pointer1);
            lowerbound = std::stoi(line.substr(pointer1, pointer2));
            pointer1 = pointer2+1;
            upperbound = std::stoi(line.substr(pointer1));
            call.deliveryWindow = std::make_pair(lowerbound, upperbound);

            problem.calls.push_back(call);
        }
        // Sort the vector based on the call index (call index might not match vector index)
        std::sort(problem.calls.begin(), problem.calls.end(), [ ](const Call& lhs, const Call& rhs) {
            return lhs.index < rhs.index;
        });

        // Close the file after finished reading
        file.close();
    } else {
        // If the file is not open, something went wrong
        std::cerr << "Error: Couldn't open data file '" << path << "'" << std::endl;
    }

    // Return the problem instance
    return problem;
}