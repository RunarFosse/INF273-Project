#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "problem.h"
#include "solution.h"
#include "operator.h"

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
     * @note Endline has to be included if intended. This function only flushes output buffer.
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
     * @param instance Name of the instance ran
     * @param algorithm Name of the algorithm ran
     * @param averageObjective Average objective over all experiments
     * @param bestObjective Best objective of all experiments
     * @param improvement Percent improvement from the initial solution
     * @param averageTime Average instance running time
     */
    static void printResults(std::string instance, std::string algorithm, double averageObjective, double bestObjective, double improvement, double averageTime, Solution* bestSolution);

    /**
     * @brief Prints a title together with a progress bar to the terminal.
     * 
     * @note Only prints an ending newline character if completed == total.
     * 
     * @param title to be displayed left of loading bar
     * @param completed pieces of loading bar which are filled in
     * @param total size of loading bar
     */
    static void printProgress(std::string title, int completed, int total);

    /**
     * @brief Decide whether the cursor should display in the terminal.
     * 
     * @param show Show the cursor blinking
     */
    static void displayCursor(bool show);

    /**
     * @brief Start storing episodic information algorithm information.
     * 
     * @param instance Current instance algorithm is running
     */
    static void storeStartOfAlgorithmInformation(std::string instance);

    /**
     * @brief Store algorithm information about the current iteration.
     * 
     * @param temperature Current temperature
     * @param acceptanceProbability Current probability for acceptance
     * @param deltaValue Current iterations delta value
     * @param adaptiveOperator Operator to extract weight values from
     */
    static void storeAlgorithmInformation(int iteration, double temperature, double acceptanceProbability, int deltaValue, AdaptiveOperator* adaptiveOperator);

    /**
     * @brief End a current episode of an algorithm.
     * 
     * @param bestSolutionIteration Iteration where best solution was obtained
     */
    static void storeEndOfAlgorithmInformation(int bestSolutionIteration);

    private:
    // This is a static class, prevent class creation
    Debugger();

    /**
     * @brief Formats a double for printing.
     * 
     * @param number the number to format
     * @param decimals decimal places after formatting
     * @return Formatted string
     */
    static std::string formatDouble(double number, int decimals);

    // Different outbuffers (terminal and file)
    static bool outputChanged;
    static std::streambuf *coutbuf;
    static std::ofstream filebuf, infobuf;
};