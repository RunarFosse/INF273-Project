#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"
#include "heuristics.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng{std::random_device {}()};

    // Create a adaptive neighbourhood operator
    Operator* adaptiveOperator = new AdaptiveOperator({
        new SimilarGreedyInsert(),
        new SimilarRegretInsert(),
        new SimilarBeamInsert(),
        new CostlyGreedyInsert(),
        new CostlyRegretInsert(),
        new CostlyBeamInsert(),
        new RandomGreedyInsert(),
        new RandomRegretInsert(),
        new RandomBeamInsert(),
    });

    // Total number of experiments to run each algorithm for
    int experiments = 5;

    // Run each test case given
    Debugger::outputToFile("results_final.txt");
    InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, "Call_7_Vehicle_3", experiments, 0.01, rng, "");
    InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, "Call_18_Vehicle_5", experiments, 0.04, rng, "");
    InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, "Call_35_Vehicle_7", experiments, 2.0, rng, "");
    InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, "Call_80_Vehicle_20", experiments, 4.5, rng, "");
    InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, "Call_130_Vehicle_40", experiments, 8.45, rng, "");
    InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, "Call_300_Vehicle_90", experiments, 10.0, rng, "");

    return 0;
}