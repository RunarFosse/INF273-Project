#include <string>
#include <iostream>

#include "parser.h"

int main(int argc, char const *argv[])
{
    Problem problem = Parser::parseProblem("data/Call_7_Vehicle_3.txt");

    std::cout << "Problem solved" << std::endl;

    return 0;
}
