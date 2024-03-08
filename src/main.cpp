#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng{std::random_device {}()};

    // Create a neighbourhood operator
    Operator* neighbourOperator = new WeightedOperator({
        std::make_pair(new OneOutsource(), 0.05),
        std::make_pair(new GreedyOutsource(), 0.1),
        std::make_pair(new OneInsert(), 0.2),
        std::make_pair(new GreedyInsert(), 0.2),
        std::make_pair(new BestInsert(), 0.45),
    });

    // Run each test case given
    Debugger::outputToFile("results.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, neighbourOperator, 10, 10000, rng);

    // Verify correct implementation of operator
    //Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Debugger::printSolution(&initial);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");



    return 0;
}