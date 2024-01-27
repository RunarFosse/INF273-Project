#pragma once

#include <vector>

struct {
    int index;
    int homeNode;
    int startTime;
    int capacity;
    std::vector<int> possibleCalls;
} typedef Vehicle;

struct {
    int index;
    int originNode;
    int destinationNode;
    int size;
    int costOfNotTransporting;
    std::pair<int, int> pickupWindow;
    std::pair<int, int> deliveryWindow;
} typedef Call;

class Problem {
    public:
    int noNodes;
    int noVehicles;
    int noCalls;
    std::vector<Vehicle> vehicles;
    std::vector<Call> calls;
    // Todo: Include remaining fields
};