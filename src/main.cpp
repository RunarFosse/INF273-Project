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
    InstanceRunner::blindRandomSearch("Call_7_Vehicle_3", 10, 10000, rng);
    InstanceRunner::blindRandomSearch("Call_18_Vehicle_5", 10, 10000, rng);
    InstanceRunner::blindRandomSearch("Call_35_Vehicle_7", 10, 10000, rng);
    InstanceRunner::blindRandomSearch("Call_80_Vehicle_20", 10, 10000, rng);
    InstanceRunner::blindRandomSearch("Call_130_Vehicle_40", 10, 10000, rng);
    InstanceRunner::blindRandomSearch("Call_300_Vehicle_90", 10, 10000, rng);

    return 0;
}