#pragma once

#include <vector>

struct {
    int index;
    int homeNode;
    int startTime;
    int capacity;
} typedef Vehicle;

class Problem {
    public:
    int noNodes;
    int noVehicles;
    int noCalls;
    std::vector<Vehicle> vehicles;
    // Todo: Include remaining fields
};