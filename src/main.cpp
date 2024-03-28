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
        new MultiOutsource(),
        new RandomInsert(),
        new RandomBestInsert(),
        new GreedyBestInsert(),
    });
    Operator* weightedOperator = new WeightedOperator({
        std::make_pair(new MultiOutsource(), 0.40),
        std::make_pair(new RandomBestInsert(), 0.60),
    });

    // Run each test case given
    Debugger::outputToFile("results_uniform.txt");
    InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, uniformOperator, 10, 10000, rng, "SA-new operators (equal weights)");

    //Debugger::outputToFile("results_weighted.txt");
    //InstanceRunner::testAlgorithm(InstanceRunner::simulatedAnnealing, weightedOperator, 10, 10000, rng, "SA-new operators (tuned weights)");

    // Verify correct implementation of operator
    //Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");
    //Solution initial = Solution::initialSolution(&problem);
    //Solution solution = uniformOperator->apply(&initial, rng);
    //Debugger::printSolution(&initial);
    //Debugger::printToTerminal("Cost: " + std::to_string(initial.getCost()) + "\n");
    //Solution solution = Solution({4, 4, 7, 7, 0, 5, 5, 2, 2, 0, 1, 3, 3, 1, 0, 6, 6}, &problem);
    //Debugger::printSolution(&solution);
    //Debugger::printToTerminal("Cost: " + std::to_string(solution.getCost()) + "\n");


    //std::vector<std::pair<int, CallDetails>> ins = calculateFeasibleInsertions(5, &solution);

    //solution.move(3, 5, std::make_pair(1, 2));
    //solution.updateFeasibility(3);
    //solution.updateCost(3, 5, std::make_pair(1, 2), true, &solution);
    //solution.updateCost(2, 5, std::make_pair(0, 1), false, &solution);
    //Debugger::printSolution(&solution);
    //Debugger::printToTerminal("Cost: " + std::to_string(solution.getCost()) + "\n");

    //for (std::pair<int, CallDetails> det : ins) {
    //    Debugger::printToTerminal(std::to_string(det.first) + ", v:" + std::to_string(det.second.vehicle) + "(" + std::to_string(det.second.indices.first) + ", " +std::to_string(det.second.indices.second) + ")\n");
    //}

    return 0;
}