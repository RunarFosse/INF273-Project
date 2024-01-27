#include <string>
#include <iostream>

#include "parser.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");

    Debugger::printProblem(&problem);

    return 0;
}
