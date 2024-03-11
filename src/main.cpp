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
        new OneOutsource(),
        new GreedyOutsource(),
        new BestInsert()
    });
    Operator* weightedOperator = new WeightedOperator({
        std::make_pair(new OneOutsource(), 0.05)
    });

    // Run each test case given
    Debugger::outputToFile("results_uniform.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, uniformOperator, 10, 10000, rng);

    // Verify correct implementation of operator
    //Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Debugger::printSolution(&initial);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");



    return 0;
}