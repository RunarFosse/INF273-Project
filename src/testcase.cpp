#include "testcase.h"

void InstanceRunner::testAlgorithm(std::function<void(Operator*, std::string, int, int, std::default_random_engine&, std::string)> algorithm, Operator* neighbourOperator, int experiments, int iterations, std::default_random_engine& rng, std::string title) {
    // Hide cursor from terminal
    Debugger::displayCursor(false);
    
    // Run the given algorithm on all different testcases, given experiment and iteration count
    algorithm(neighbourOperator, "Call_7_Vehicle_3", experiments, iterations, rng, title);
    algorithm(neighbourOperator, "Call_18_Vehicle_5", experiments, iterations, rng, title);
    algorithm(neighbourOperator, "Call_35_Vehicle_7", experiments, iterations, rng, title);
    algorithm(neighbourOperator, "Call_80_Vehicle_20", experiments, iterations, rng, title);
    algorithm(neighbourOperator, "Call_130_Vehicle_40", experiments, iterations, rng, title);
    algorithm(neighbourOperator, "Call_300_Vehicle_90", experiments, iterations, rng, title);

    // Show cursor in terminal
    Debugger::displayCursor(true);
}

void InstanceRunner::blindRandomSearch(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title = "Blind Random Search") {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm  = title == "" ? "Blind Random Search" : title;

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

void InstanceRunner::localSearch(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title = "Local Search") {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm  = title == "" ? "Local Search" : title;

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
            Solution solution = neighbourOperator->apply(&bestSolution, j, rng);

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

void InstanceRunner::simulatedAnnealing(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title = "Simulated Annealing") {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm = title == "" ? "Simulated Annealing" : title;

    // Create a timer object
    Timer timer = Timer(algorithm + ": " + instance, experiments);
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    int foundBestIteration = 0;

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
            Solution solution = neighbourOperator->apply(&incumbent, w, rng);

            if (!solution.isFeasible()) {
                continue;
            }
            
            double deltaE = solution.getCost() - incumbent.getCost();
            if (deltaE < 0) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                    foundBestIteration = w;
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
        for (int j = warmupIterations; j < iterations; j++) {
            // Generate a new neighbour solution
            Solution solution = neighbourOperator->apply(&incumbent, j, rng);

            if (!solution.isFeasible()) {
                continue;
            }

            double deltaE = solution.getCost() - incumbent.getCost();
            if (deltaE < 0) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                    foundBestIteration = j;
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
        if (true) {
            Debugger::printSolution(&bestSolution);
            std::cout << "Cost: " << std::to_string(bestSolution.getCost());
            bestSolution.invalidateCache();
            std::cout << " Actual: " << std::to_string(bestSolution.getCost()) << ", found after iteration " << std::to_string(foundBestIteration) << std::endl;
        }
    }

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Retrieve runtime from timer
    double averageTime = timer.retrieve();

    // Print the results to standard output
    Debugger::printResults(instance, algorithm, averageObjective, bestSolutionOverall.getCost(), improvement, averageTime, &bestSolutionOverall);
}

void InstanceRunner::generalAdaptiveMetaheuristic(Operator* neighbourOperator, std::string instance, int experiments, int iterations, std::default_random_engine& rng, std::string title) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm = title == "" ? "General Adaptive Metaheuristic" : title;

    // Decide if we output extra information
    bool outputAlgorithmInformation = false;
    bool printEscapes = true;

    if (outputAlgorithmInformation) {
        Debugger::storeStartOfAlgorithmInformation(instance);
    }

    // Create a timer object
    Timer timer = Timer(algorithm + ": " + instance, experiments);
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    int iterfound = 0, escapeCondition = 1000;
    
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

        int lastIncumbantChange = 0;

        for (int w = 0; w < warmupIterations; w++) {
            Solution solution = neighbourOperator->apply(&incumbent, w, rng);

            if (!solution.isFeasible()) {
                continue;
            }
            
            int deltaE = solution.getCost() - incumbent.getCost();
            if (deltaE < 0) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                    iterfound = w;
                }
                lastIncumbantChange = w;
            } else {
                if (random(rng) < explorationProbability) {
                    incumbent = solution;
                    lastIncumbantChange = w;
                }
                deltaAverage += (deltaE - deltaAverage) / updates;
                updates++;
            }

            if (outputAlgorithmInformation) {
                Debugger::storeAlgorithmInformation(w, 0, explorationProbability, deltaE, (AdaptiveOperator*) neighbourOperator);
            }
        }

        // Calculate annealing values
        double temperature_0 = -deltaAverage / log(explorationProbability);
        double alpha = pow(temperature_f / temperature_0, 1.0 / (iterations - warmupIterations));

        double temperature = temperature_0;

        // Run iterations per experiment
        for (int j = warmupIterations; j < iterations; j++) {
            // If long since successful modification of incumbant, run escape algorithm
            if (j - lastIncumbantChange >= escapeCondition) {
                if (printEscapes)
                    Debugger::printToTerminal("Applying escape at iteration: " + std::to_string(j) + "\n");
                // Copy incumbant to not make changes to best solution
                incumbent = incumbent.copy();

                // Then perform many small steps with most diversifying operator
                for (int k = 0; k < 50; k++) {
                    int lowerbound = std::max(1, incumbent.problem->noCalls / 25);
                    int upperbound = std::max(3, incumbent.problem->noCalls / 5);
                    int callsToRemove = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

                    // Random removal
                    std::vector<int> removedCalls = removeRandom(callsToRemove, &incumbent, rng);

                    // Greedy insertion
                    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
                    // Tiny chance to insert random
                    if (random(rng) < 0.03) {
                        insertRandom(callIndices, &incumbent, rng);
                    } else {
                        insertGreedy(callIndices, &incumbent);
                    }

                    if (incumbent.getCost() < bestSolution.getCost()) {
                        bestSolution = incumbent.copy();
                        iterfound = j;
                        if (printEscapes)
                            Debugger::printToTerminal("Found new optimal during escape! Cost: " + std::to_string(incumbent.getCost()) + "\n");
                    }
                }
                
                lastIncumbantChange = j;
            }

            // Generate a new neighbour solution
            Solution solution = neighbourOperator->apply(&incumbent, j, rng);

            if (!solution.isFeasible()) {
                continue;
            }

            int deltaE = solution.getCost() - incumbent.getCost();
            if (deltaE < 0) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                    iterfound = j;
                    lastIncumbantChange = j;
                    if (printEscapes)
                        Debugger::printToTerminal("Found new optimal at iteration: " + std::to_string(j) +". Cost: " + std::to_string(incumbent.getCost()) + "\n");
                }
            } else if (random(rng) < exp(-deltaE / temperature)) {
                incumbent = solution;
            }

            if (outputAlgorithmInformation) {
                Debugger::storeAlgorithmInformation(j, temperature, exp(-deltaE / temperature), deltaE, (AdaptiveOperator*) neighbourOperator);
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
        if (true) {
            Debugger::printSolution(&bestSolution);
            std::cout << "Cost: " << std::to_string(bestSolution.getCost());
            bestSolution.invalidateCache();
            std::cout << " Actual: " << std::to_string(bestSolution.getCost()) << ", found after iteration " << std::to_string(iterfound) << std::endl;
        }

        if (outputAlgorithmInformation) {
            Debugger::storeEndOfAlgorithmInformation(iterfound);
        }
    }

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Retrieve runtime from timer
    double averageTime = timer.retrieve();

    // Print the results to standard output
    Debugger::printResults(instance, algorithm, averageObjective, bestSolutionOverall.getCost(), improvement, averageTime, &bestSolutionOverall);
}

AlgorithmInformation InstanceRunner::finalAdaptiveMetaheuristic(Operator* neighbourOperator, std::string instance, int experiments, double time, std::default_random_engine& rng, std::string title) {
    // Parse the given test instance
    Problem problem = Parser::parseProblem("data/" + instance + ".txt");
    std::string algorithm = title == "" ? "Final Adaptive Metaheuristic" : title;

    // Store information about each episode
    std::vector<EpisodeInformation> episodes;

    // Debug print escape information
    bool printEscapes = false;

    // Create a timer object
    Timer timer = Timer(algorithm + ": " + instance, experiments);

    // Compute a timebased deadline in seconds
    double deadline = time * 60;
    
    Solution bestSolutionOverall = Solution::initialSolution(&problem);
    double initialObjective = bestSolutionOverall.getCost();
    double averageObjective = 0;

    double timefound = 0.0;
    int iterfound = 0, escapeCondition = 7500;
    double dMultiplier = 1.0 / std::pow(problem.noCalls, 2.0 / 3.0);

    // Declare uniform [0, 1) distribution
    std::uniform_real_distribution<double> random(0, 1);

    // Run the experiments
    for (int i = 0; i < experiments; i++) {
        // Start timer
        timer.start();

        // Initialize the initial solution as the "current best"
        Solution bestSolution = Solution::initialSolution(&problem);
        Solution incumbent = Solution::initialSolution(&problem);

        // Store last time new best is found
        int lastBestFound = 0;

        // Run iterations per experiment
        int totalIterations = 0;
        for (int j = 0; timer.check() < deadline; j++) {
            totalIterations = j;

            // If long since successful modification of incumbant, run escape algorithm
            if (j - lastBestFound >= escapeCondition) {
                if (printEscapes) {
                    Debugger::printToTerminal("Applying escape at iteration: " + std::to_string(j) + "\n");
                }

                // Copy incumbant to not make changes to best solution
                if (random(rng) < 2.0 / incumbent.problem->noCalls) {
                    incumbent = bestSolution.copy();
                } else {
                    incumbent = incumbent.copy();
                }

                // Then perform many small steps with most diversifying operator
                for (int k = 0; k < 25; k++) {
                    int lowerbound = std::max(1, (int)std::floor(incumbent.problem->noCalls / 50.0));
                    int upperbound = std::max(2, (int)std::ceil(incumbent.problem->noCalls / 15.0));
                    int callsToRemove = std::uniform_int_distribution<int>(lowerbound, upperbound)(rng);

                    // Random removal
                    std::vector<int> removedCalls = removeRandom(callsToRemove, &incumbent, rng);

                    // Greedy insertion
                    std::set<int> callIndices(removedCalls.begin(), removedCalls.end());
                    // Tiny chance to insert random
                    insertGreedy(callIndices, &incumbent);

                    if (incumbent.getCost() < bestSolution.getCost()) {
                        bestSolution = incumbent.copy();
                        iterfound = j;
                        timefound = timer.check();
                        if (printEscapes) {
                            Debugger::printToTerminal("Found new optimal during escape! Cost: " + std::to_string(incumbent.getCost()) + "\n");
                        }
                    }
                }
                
                lastBestFound = j;
            }

            // Generate a new neighbour solution
            Solution solution = neighbourOperator->apply(&incumbent, j, rng);

            if (!solution.isFeasible()) {
                continue;
            }

            double d = dMultiplier * std::max(std::pow((deadline - timer.check()) / deadline, 2.0), 0.01) * bestSolution.getCost();
            if (solution.getCost() < bestSolution.getCost() + d) {
                incumbent = solution;
                if (incumbent.getCost() < bestSolution.getCost()) {
                    bestSolution = incumbent;
                    iterfound = j;
                    timefound = timer.check();
                    lastBestFound = j;
                    if (printEscapes) {
                        Debugger::printToTerminal("Found new optimal at iteration: " + std::to_string(j) +". Cost: " + std::to_string(incumbent.getCost()) + "\n");
                    }
                }
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

        // Store episode information
        int greedyCost = bestSolution.getCost();
        bestSolution.invalidateCache();
        int actualCost = bestSolution.getCost();
        episodes.push_back({bestSolution, greedyCost, actualCost, iterfound, timefound, totalIterations});
    }

    // Calculate the improvement from the initial solution
    double improvement = 100 * (initialObjective - bestSolutionOverall.getCost()) / initialObjective;

    // Retrieve runtime from timer
    double averageTime = timer.retrieve();

    // Store and return algorithm information
    AlgorithmInformation information = {instance, algorithm, averageObjective, bestSolutionOverall, improvement, averageTime, episodes};
    return information;
}