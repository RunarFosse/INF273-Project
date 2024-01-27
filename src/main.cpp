#include <string>
#include <iostream>

#include "parser.h"

int main(int argc, char const *argv[])
{
    std::string out = Parser::parseProblem("problem.txt");
    
    std::cout << out << std::endl;

    return 0;
}
