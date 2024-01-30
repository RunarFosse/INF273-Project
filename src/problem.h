#pragma once

#include <vector>

// TODO: Use specific structs instead of std::pair

struct {
    int homeNode;
    int startTime;
    int capacity;
    std::vector<int> possibleCalls;
    std::vector<std::vector<std::pair<int, int>>> routeTimeCost;
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> callTimeCost;
} typedef Vehicle;

struct {
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
};