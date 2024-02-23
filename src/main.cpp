#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng{std::random_device {}()};

    // Create a neighbourhood operator
    Operator* neighbourOperator = new OneInsert();

    // Run each test case given
    Debugger::outputToFile("results.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, neighbourOperator, 10, 10000, rng);

    return 0;
}