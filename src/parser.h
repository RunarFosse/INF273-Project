#include <string>

#include "problem.h"

class Parser {
    public:

    static Problem parseProblem(std::string path);

    private:

    // This is a static class, prevent class creation
    Parser();
};