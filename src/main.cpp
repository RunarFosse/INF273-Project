#include <string>
#include <iostream>

#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    // Initialize a random number generator
    std::default_random_engine rng = std::default_random_engine{std::random_device {}()};

    // Run the first test case
    InstanceRunner::blindRandomSearch("Call_7_Vehicle_3", 10, 10000, rng);

    return 0;
}
