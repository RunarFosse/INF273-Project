#include <iostream>
#include <string>

#include "problem.h"
#include "solution.h"

class Debugger {
    public:
    /**
     * @brief Print the problem description to standard output.
     * 
     * @param problem Pointer to problem instance
     */
    static void printProblem(Problem* problem, bool printMap = true);

    /**
     * @brief Print the given solution representation.
     * 
     * @param solution Pointer to the solution instance
     */
    static void printSolution(Solution* solution);

    private:
    // This is a static class, prevent class creation
    Debugger();
};