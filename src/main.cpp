#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng = std::default_random_engine{std::random_device {}()};

    // Redirect standard output to 'results.txt'
    Debugger::outputToFile("results.txt");

    // Run each test case given
    InstanceRunner::testAlgorithm(InstanceRunner::blindRandomSearch, 10, 10000, rng);

    return 0;
}