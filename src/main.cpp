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

    return 0;
}
