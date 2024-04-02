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
        new CostlyGreedyInsert(),
        new CostlyRegretInsert(),
        new RandomGreedyInsert(),
        new RandomRegretInsert(),
    });

    // Run each test case given
    Debugger::outputToFile("results.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::generalAdaptiveMetaheuristic, adaptiveOperator, 10, 10000, rng, "Adaptive Algorithm");

    // Verify correct implementation of operator
    //Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Solution solution = uniformOperator->apply(&initial, rng);
    //Debugger::printSolution(&initial);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");


    //removeSimilar(2, &initial, rng);
    //Debugger::printSolution(&initial);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");

    //for (auto [callIndex, relatedness] : problem.calls[5-1].similarities) {
    //    Debugger::printToTerminal("(" + std::to_string(callIndex) + ", " + std::to_string(relatedness) + ")\n");
    //}

    return 0;
}