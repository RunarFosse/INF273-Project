#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "problem.h"
#include "solution.h"

class Debugger {
    public:
    /**
     * @brief Change standard output to file.
     * 
     * @param path Path of the given output file
     */
    static void outputToFile(std::string path);

    /**
     * @brief Prints a message to the terminal.
     * 
     * @param message 
     */
    static void printToTerminal(std::string message);

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

    /**
     * @brief Print the results of running a test instance.
     * 
     * @param instanceName Name of the instance ran
     * @param averageObjective Average objective over all experiments
     * @param bestObjective Best objective of all experiments
     * @param improvement Percent improvement from the initial solution
     * @param runningTime Total instance running time
     */
    static void printResults(std::string instanceName, double averageObjective, double bestObjective, double improvement, double runningTime, Solution* bestSolution);

    private:
    // This is a static class, prevent class creation
    Debugger();

    /**
     * @brief Formats a double for printing.
     * 
     * @param number to format
     * @param decimals places after formatting
     * @return Formatted string
     */
    static std::string formatDouble(double number, int decimals);

    // Different outbuffers (terminal and file)
    static bool outputChanged;
    static std::streambuf *coutbuf;
    static std::ofstream filebuf;
};