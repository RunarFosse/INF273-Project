#pragma once

#include <vector>

// TODO: Optimize storage (remove storing of index (all indicies are implicit))
//       Use specific structs instead of std::pair

typedef std::vector<std::vector<std::pair<int, int>>> Map;

struct {
    int index;
    int homeNode;
    int startTime;
    int capacity;
    std::vector<int> possibleCalls;
    Map map;
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