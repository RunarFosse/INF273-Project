#include <string>
#include <iostream>

#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");

    //Solution solution = Solution::initialSolution(&problem);
    //Solution solution = Solution({0, 2, 2, 0, 1, 5, 5, 3, 1, 3, 0, 7, 4, 6, 7, 4, 6}, &problem);

    std::default_random_engine rng = std::default_random_engine{std::random_device {}()};
    
    for (int i = 0; i < 10000; i++) {
        Solution solution = Solution::randomSolution(&problem, rng);

        if (solution.isFeasible()) {
            Debugger::printSolution(&solution);
            std::cout << "Solution is feasible!" << std::endl;
            std::cout << "Total cost: " << std::to_string(solution.getCost()) << std::endl << std::endl;
        }
    }

    return 0;
}
