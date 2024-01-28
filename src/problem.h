#pragma once

#include <vector>

// TODO: Optimize storage (remove storing of index (all indicies are implicit))
//       Use specific structs instead of std::pair

struct {
    int index;
    int homeNode;
    int startTime;
    int capacity;
    std::vector<int> possibleCalls;
    std::vector<std::vector<std::pair<int, int>>> routeTimeCost;
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> callTimeCost;
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