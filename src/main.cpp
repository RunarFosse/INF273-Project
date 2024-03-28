#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"


int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng{std::random_device {}()};

    // Create a uniform and weighted neighbourhood operator
    Operator* uniformOperator = new UniformOperator({
        new MultiOutsource(),
        new RandomInsert(),
        new RandomBestInsert(),
        new GreedyBestInsert(),
    });
    Operator* weightedOperator = new WeightedOperator({
        std::make_pair(new MultiOutsource(), 0.40),
        std::make_pair(new RandomBestInsert(), 0.60),
    });

    // Run each test case given
    Debugger::outputToFile("results_uniform.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, uniformOperator, 10, 10000, rng, "SA-new operators (equal weights)");

    //Debugger::outputToFile("results_weighted.txt");
    //InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, weightedOperator, 10, 10000, rng, "SA-new operators (tuned weights)");

    // Verify correct implementation of operator
    //Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Solution solution = uniformOperator->apply(&initial, rng);
    //Debugger::printSolution(&initial);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");

    //for (auto [callIndex, relatedness] : problem.calls[4-1].similarities) {
    //    Debugger::printToTerminal("(" + std::to_string(callIndex) + ", " + std::to_string(relatedness) + ")\n");
    //}

    return 0;
}