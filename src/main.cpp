#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng = std::default_random_engine{std::random_device {}()};

    // Run each test case given
    Debugger::outputToFile("results_ls.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::localSearch, false, 10, 10000, rng);

    Debugger::outputToFile("results_sa.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, false, 10, 10000, rng);

    return 0;
}