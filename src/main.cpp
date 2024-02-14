#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng = std::default_random_engine{std::random_device {}()};

    // Run each test case given
    //InstanceRunner::testAlgorithm(InstanceRunner::blindRandomSearch, 10, 10000, rng);

    Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    Solution initial = Solution::initialSolution(&problem);
    Solution neighbour = initial.getNeighbour(rng);

    Debugger::printSolution(&initial);
    Debugger::printSolution(&neighbour);

    return 0;
}