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
        new MediumBestInsert(),
        new LowBestInsert(),
        new HighBestInsert(),
    });
    Operator* weightedOperator = new WeightedOperator({
        std::make_pair(new OneOutsource(), 0.07),
        std::make_pair(new GreedyOutsource(), 0.03),
        std::make_pair(new LowBestInsert(), 0.30),
        std::make_pair(new MediumBestInsert(), 0.40),
        std::make_pair(new HighBestInsert(), 0.20),
    });

    // Run each test case given
    Debugger::outputToFile("results_uniform.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, uniformOperator, 10, 10000, rng, "SA-new operators (equal weights)");

    //Debugger::outputToFile("results_weighted.txt");
    //InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, weightedOperator, 10, 10000, rng, "SA-new operators (tuned weights)");

    return 0;
}