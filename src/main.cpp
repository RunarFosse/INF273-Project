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
        //new MultiOutsource(),
        new OneInsert(),
        //new ConstantBestInsert(),
        //new LowBestInsert(),
        //new HighBestInsert(),
    });
    Operator* weightedOperator = new WeightedOperator({
        std::make_pair(new MultiOutsource(), 0.40),
        std::make_pair(new HighBestInsert(), 0.60),
    });

    // Run each test case given
    Debugger::outputToFile("results_uniform.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, uniformOperator, 10, 10000, rng, "SA-new operators (equal weights)");

    //Debugger::outputToFile("results_weighted.txt");
    //InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, weightedOperator, 10, 10000, rng, "SA-new operators (tuned weights)");

    // Verify correct implementation of operator
    //Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Solution solution = uniformOperator->apply(initial, rng);
    //Debugger::printSolution(&initial);
    //Debugger::printSolution(&solution);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");
    //Debugger::printToTerminal("Cost: " + std::to_string(solution.getCost()) + "\n");

    return 0;
}