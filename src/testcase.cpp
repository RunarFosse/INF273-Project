#include "testcase.h"

void InstanceRunner::testAlgorithm(std::function<void(std::string, int, int, std::default_random_engine&)> algorithm, int experiments, int iterations, std::default_random_engine& rng) {
    // Hide cursor from terminal
    Debugger::displayCursor(false);
    
    // Run the given algorithm on all different testcases, given experiment and iteration count
    algorithm("Call_7_Vehicle_3", experiments, iterations, rng);
    algorithm("Call_18_Vehicle_5", experiments, iterations, rng);
    algorithm("Call_35_Vehicle_7", experiments, iterations, rng);
    //algorithm("Call_80_Vehicle_20", experiments, iterations, rng);
    //algorithm("Call_130_Vehicle_40", experiments, iterations, rng);
    //algorithm("Call_300_Vehicle_90", experiments, iterations, rng);

    // Show cursor in terminal
    Debugger::displayCursor(true);
}

void InstanceRunner::blindRandomSearch(std::string instance, int experiments, int iterations, std::default_random_engine& rng) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm = "Blind Random Search";

    // Create a timer object
    Timer timer(algorithm + ": " + instance, experiments);

    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    // Run the experiments
    for (int i = 0; i < experiments; i++) {
        // Start timer
        timer.start();

        // Initialize the initial solution as the "current best"
        Solution bestSolution = Solution::initialSolution(&problem);
        // Run iterations per experiment
        for (int j = 0; j < iterations; j++) {
            // Generate a new random solution
            Solution solution = Solution::randomSolution(&problem, rng);

            // Check if the solution is feasible and, if so, check if it is better than the current best
            if (solution.isFeasible() && solution.getCost() < bestSolution.getCost()) {
                bestSolution = solution;
            }
        }

        // Capture current time
        timer.capture();

        // At the end of the experiment, count the current best cost towards the average cost
        averageObjective += (double)bestSolution.getCost() / experiments;
        // and check if it is better than the current best overall solution
        if (bestSolution.getCost() < bestSolutionOverall.getCost()) {
            bestSolutionOverall = bestSolution;
        }
    }

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Retrieve runtime from timer
    double averageTime = timer.retrieve();

    // Print the results to standard output
    Debugger::printResults(instance, algorithm, averageObjective, bestSolutionOverall.getCost(), improvement, averageTime, &bestSolutionOverall);
}

void InstanceRunner::localSearch(std::string instance, int experiments, int iterations, std::default_random_engine& rng) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm = "Local Search";

    // Create a timer object
    Timer timer(algorithm + ": " + instance, experiments);
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;


    // Run the experiments
    for (int i = 0; i < experiments; i++) {
        // Start timer
        timer.start();

        // Initialize the initial solution as the "current best"
        Solution bestSolution = Solution::initialSolution(&problem);
        // Run iterations per experiment
        for (int j = 0; j < iterations; j++) {
            // Generate a new neighbour solution
            Solution solution = bestSolution.getNeighbour(rng);

            // Check if the solution is feasible and, if so, check if it is better than the current best
            if (solution.isFeasible() && solution.getCost() < bestSolution.getCost()) {
                bestSolution = solution;
            }
        }

        // Capture current time
        timer.capture();

        // At the end of the experiment, count the current best cost towards the average cost
        averageObjective += (double)bestSolution.getCost() / experiments;
        // and check if it is better than the current best overall solution
        if (bestSolution.getCost() < bestSolutionOverall.getCost()) {
            bestSolutionOverall = bestSolution;
        }
    }

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Retrieve runtime from timer
    double averageTime = timer.retrieve();

    // Print the results to standard output
    Debugger::printResults(instance, algorithm, averageObjective, bestSolutionOverall.getCost(), improvement, averageTime, &bestSolutionOverall);
}

void InstanceRunner::simulatedAnnealing(std::string instance, int experiments, int iterations, std::default_random_engine& rng) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm = "Simulated Annealing";

    // Create a timer object
    Timer timer(algorithm + ": " + instance, experiments);
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    // Declare probability of exploring during "warmup" period
    double explorationProbability = 0.8;
    int warmupIterations = iterations / 100;

    // Declare uniform [0, 1) distribution
    std::uniform_real_distribution<double> random(0, 1);

    // Run the experiments
    for (int i = 0; i < experiments; i++) {
        // Start timer
        timer.start();

        // Initialize the initial solution as the "current best"
        Solution bestSolution = Solution::initialSolution(&problem);
        Solution incumbent = Solution::initialSolution(&problem);

        // Declare final temperature (end condition)
        double temperature_f = 0.1, deltaAverage = 0.0;
        int updates = 1;

        for (int w = 0; w < warmupIterations; w++) {
            Solution solution = incumbent.getNeighbour(rng);

            if (!solution.isFeasible()) {
                continue;
            }
            
            double deltaE = solution.getCost() - incumbent.getCost();
            if (deltaE < 0) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                }
            } else {
                if (random(rng) < explorationProbability) {
                    incumbent = solution;
                }
                deltaAverage += (deltaE - deltaAverage) / updates;
                updates++;
            }
        }

        // Calculate annealing values
        double temperature_0 = -deltaAverage / log(explorationProbability);
        double alpha = pow(temperature_f / temperature_0, 1.0 / (iterations - warmupIterations));

        double temperature = temperature_0;

        // Run iterations per experiment
        for (int j = 0; j < iterations - warmupIterations; j++) {
            // Generate a new neighbour solution
            Solution solution = incumbent.getNeighbour(rng);

            if (!solution.isFeasible()) {
                continue;
            }

            double deltaE = solution.getCost() - incumbent.getCost();
            if (deltaE < 0) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                }
            } else if (random(rng) < exp(-deltaE / temperature)) {
                incumbent = solution;
            }

            // Lower the temperature
            temperature *= alpha;
        }

        // Capture current time
        timer.capture();

        // At the end of the experiment, count the current best cost towards the average cost
        averageObjective += (double)bestSolution.getCost() / experiments;
        // and check if it is better than the current best overall solution
        if (bestSolution.getCost() < bestSolutionOverall.getCost()) {
            bestSolutionOverall = bestSolution;
        }
    }

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Retrieve runtime from timer
    double averageTime = timer.retrieve();

    // Print the results to standard output
    Debugger::printResults(instance, algorithm, averageObjective, bestSolutionOverall.getCost(), improvement, averageTime, &bestSolutionOverall);
}