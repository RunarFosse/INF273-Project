#include "debug.h"

// Set values of private static variables
bool Debugger::outputChanged = false;
std::streambuf *Debugger::coutbuf = std::cout.rdbuf();
std::ofstream Debugger::filebuf;

void Debugger::outputToFile(std::string path) {
    outputChanged = true;

    // Redirect standard output to 'path'
    filebuf = std::ofstream(path);
    std::cout.rdbuf(filebuf.rdbuf());
}

void Debugger::printToTerminal(std::string message) {
    // Redirect standard output to terminal
    if (outputChanged) {
        std::cout.rdbuf(coutbuf);
    }
    // Print the message
    std::cout << message << std::flush;
    // Redirect standard output back to what it was
    if (outputChanged) {
        std::cout.rdbuf(filebuf.rdbuf());
    }
}

void Debugger::printProblem(Problem* problem, bool printMap) {
    std::cout << "Number of nodes: " << std::to_string(problem->noNodes) << std::endl;
    std::cout << "Number of vehicles: " << std::to_string(problem->noVehicles) << std::endl;
    std::cout << "Number of calls: " << std::to_string(problem->noCalls) << std::endl;

    std::cout << "For each vehicle: vehicle index, home node, starting time, capacity" << std::endl;
    for (int i = 1; i <= problem->noVehicles; i++) {
        std::cout << std::to_string(i) << ", ";
        std::cout << std::to_string(problem->vehicles[i-1].homeNode) << ", ";
        std::cout << std::to_string(problem->vehicles[i-1].startTime) << ", ";
        std::cout << std::to_string(problem->vehicles[i-1].capacity) << std::endl;
    }

    std::cout << "For each vehicle: vehicle index, and then a list of calls that can be transported using that vehicle" << std::endl;
    for (int i = 1; i <= problem->noVehicles; i++) {
        std::cout << std::to_string(i) << ", [ ";
        for (int possibleCall : problem->vehicles[i-1].possibleCalls) {
            std::cout << std::to_string(possibleCall) << " ";
        }
        std::cout << "]" << std::endl;
    }

    std::cout << "For each call: call index, origin node, destination node, size, cost of not transporting, timewindow for pickup, timewindow for delivery" << std::endl;
    for (int i = 1; i <= problem->noCalls; i++) {
        std::cout << std::to_string(i) << ", ";
        std::cout << std::to_string(problem->calls[i-1].originNode) << ", ";
        std::cout << std::to_string(problem->calls[i-1].destinationNode) << ", ";
        std::cout << std::to_string(problem->calls[i-1].size) << ", ";
        std::cout << std::to_string(problem->calls[i-1].costOfNotTransporting) << ", ";
        std::cout << "(" << std::to_string(problem->calls[i-1].pickupWindow.start) << ", ";
        std::cout << std::to_string(problem->calls[i-1].pickupWindow.end) << "), ";
        std::cout << "(" << std::to_string(problem->calls[i-1].deliveryWindow.start) << ", ";
        std::cout << std::to_string(problem->calls[i-1].deliveryWindow.end) << ")" << std::endl;
    }

    if (printMap) {
        std::cout << "For each vehicle's node edge: vehicle index, origin node, destination node, travel time (in hours), travel cost (in Euro)" << std::endl;
        for (int i = 1; i <= problem->noNodes; i++) {
            for (int j = 1; j <= problem->noNodes; j++) {
                for (int k = 1; k <= problem->noVehicles; k++) {
                    std::cout << std::to_string(k) << ", ";
                    std::cout << std::to_string(i) << ", ";
                    std::cout << std::to_string(j) << ", ";
                    std::cout << std::to_string(problem->vehicles[k-1].routeTimeCost[i-1][j-1].time) << ", ";
                    std::cout << std::to_string(problem->vehicles[k-1].routeTimeCost[i-1][j-1].cost) << std::endl;
                }
            }
        }
    }

    std::cout << "For each vehicle's call: vehicle, call, origin node time (in hours), origin node costs (in Euro), destination node time (in hours), destination node costs (in Euro)" << std::endl;
    for (int i = 1; i <= problem->noVehicles; i++) {
        for (int j = 1; j <= problem->noCalls; j++) {
            std::cout << std::to_string(i) << ", ";
            std::cout << std::to_string(j) << ", ";
            std::cout << "(" << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].first.time) << ", ";
            std::cout << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].first.cost) << "), ";
            std::cout << "(" << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].second.time) << ", ";
            std::cout << std::to_string(problem->vehicles[i-1].callTimeCost[j-1].second.cost) << ")" << std::endl;
        }
    }
};

void Debugger::printSolution(Solution* solution) {
    std::cout << "[";

    for (int i = 0; i < solution->representation.size(); i++) {
        std::cout << std::to_string(solution->representation[i]);
        if (i < solution->representation.size()-1) {
            std::cout << ", ";
        }
    }

    std::cout << "]" << std::endl;
};

void Debugger::printResults(std::string instanceName, double averageObjective, double bestObjective, double improvement, double runningTime, Solution* bestSolution) {
    std::cout << std::endl;

    std::cout << "Instance name     :    " << instanceName << std::endl;
    std::cout << "Average objective :    " << formatDouble(averageObjective, 2) << std::endl;
    std::cout << "Best objective    :    " << formatDouble(bestObjective, 2) << std::endl;
    std::cout << "Improvement (%)   :    " << std::to_string(improvement) << " %" << std::endl;
    std::cout << "Running time (s)  :    " << formatDouble(runningTime, 3) << " s" << std::endl;

    std::cout << "Best solution     :    ";
    Debugger::printSolution(bestSolution);
    
    // "ready-to-copypaste"
    std::cout << "LaTeX copypaste   :    " << "& ";
    std::cout << formatDouble(averageObjective, 2) << " & ";
    std::cout << formatDouble(bestObjective, 2) << " & ";
    std::cout << std::to_string(improvement) << " \% & ";
    std::cout << formatDouble(runningTime, 3) << " s\\" << std::endl;

    std::cout << std::endl;
}

void Debugger::printProgress(std::string title, int completed, int total) {
    // Print title to terminal
    Debugger::printToTerminal("\r" + title);

    // End character (either newline or carriage return)
    char terminator = (completed == total) ? '\n' : ' ';

    // Create the bar
    std::string bar = "  [";
    bar.reserve(total);
    while (total > 0) {
        if (completed > 0) {
            bar.push_back('#');
            completed--;
        } else {
            bar.push_back('.');
        }
        total--;
    }

    bar.push_back(']');
    bar.push_back(terminator);

    // Output to terminal and set cursor back to start of line
    Debugger::printToTerminal(bar);
}

void Debugger::displayCursor(bool show) {
    // Uses ansi code to display or hide terminal cursor
    std::string code = show ? "\e[?25h" : "\e[?25l";
    Debugger::printToTerminal(code);
}

std::string Debugger::formatDouble(double number, int decimals) {
    // Stringify number
    std::string string = std::to_string(number);

    // Prevent decimals being set to negative
    if (decimals < 0) {
        decimals = 0;
    }

    // Remove characters such that wanted number of decimals remain
    for (int i = 0; i < 6 - decimals; i++) {
        string.pop_back();
    }

    return string;
}