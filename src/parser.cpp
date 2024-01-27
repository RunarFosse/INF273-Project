#include "parser.h"

#include <iostream>

std::string Parser::parseProblem(std::string path) {
    std::cout << "Reading file " << path << std::endl;

    for (int i = 1; i < 4; i++) {
        for (int j = 0; j < 400000000; j++);
        std::cout << std::to_string(i) << " " << std::flush;
    }
    std::cout << std::endl;

    return "Success!";
}