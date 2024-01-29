#include <string>
#include <iostream>

#include "solution.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");

    Solution solution = Solution::initialSolution(&problem);
    
    Debugger::printSolution(&solution);
    if (solution.isFeasible()) {
        std::cout << "Solution is feasible!" << std::endl;
    } else {
        std::cout << "Solution is infeasible." << std::endl;
    }

    return 0;
}
