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
        }

        // Parse the number of calls
        file.ignore(LONG_MAX, '\n');
        std::getline(file, line);
        problem.noCalls = std::stoi(line);

        // Close the file after finished reading
        file.close();
    } else {
        // If the file is not open, something went wrong
        std::cerr << "Error: Couldn't open data file '" << path << "'" << std::endl;
    }

    // Return the problem instance
    return problem;
}