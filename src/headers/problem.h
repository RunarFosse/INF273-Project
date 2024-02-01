#pragma once

#include <vector>

struct {
    int time;
    int cost;
} typedef TimeCost;

struct {
    int start;
    int end;
} typedef Interval;

struct {
    int homeNode;
    int startTime;
    int capacity;
    std::vector<int> possibleCalls;
    std::vector<std::vector<TimeCost>> routeTimeCost;
    std::vector<std::pair<TimeCost, TimeCost>> callTimeCost;
} typedef Vehicle;

struct {
    int originNode;
    int destinationNode;
    int size;
    int costOfNotTransporting;
    Interval pickupWindow;
    Interval deliveryWindow;
    std::vector<int> possibleVehicles;
} typedef Call;

class Problem {
    public:
    int noNodes;
    int noVehicles;
    int noCalls;
    std::vector<Vehicle> vehicles;
    std::vector<Call> calls;
};