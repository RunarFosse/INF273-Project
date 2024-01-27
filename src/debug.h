#include <iostream>
#include <string>

#include "problem.h"

class Debugger {
    public:
    /**
     * @brief Print the problem description to standard output.
     * 
     * @param problem Pointer to problem instance
     */
    static void printProblem(Problem* problem);

    private:
    // This is a static class, prevent class creation
    Debugger();
};