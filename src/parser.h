#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "problem.h"

class Parser {
    public:
    /**
     * @brief Parse a given problem data file into a problem instance.
     * 
     * @param path Problem data file
     * @return Problem instance of given data file
     */
    static Problem parseProblem(std::string path);

    private:
    // This is a static class, prevent class creation
    Parser();
};