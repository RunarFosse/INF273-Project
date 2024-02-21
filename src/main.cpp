#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng{std::random_device {}() + 45};

    // Run each test case given
    Debugger::outputToFile("results_ls.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::localSearch, 10, 10000, rng);

    Debugger::outputToFile("results_sa.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, 10, 10000, rng);

    // Verify correct implementation of 1-insert
    //Problem problem = Parser::parseProblem("data/Call_18_Vehicle_5.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Solution neighbour = initial.getNeighbour(rng);
    //Debugger::printSolution(&initial);
    //Debugger::printSolution(&neighbour);

    return 0;
}