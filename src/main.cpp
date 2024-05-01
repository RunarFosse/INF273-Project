#include <thread>
#include <mutex>
#include <chrono>

#include "testcase.h"
#include "solution.h"
#include "parser.h"
#include "debug.h"
#include "heuristics.h"

int main(int argc, char const *argv[])
{
    // Total number of experiments to run each algorithm for
    int experiments = 10;

    // Declare each instance to run
    std::vector<std::pair<std::string, double>> instances = {{"Call_7_Vehicle_3", 2.0}, 
                                                          {"Call_18_Vehicle_5", 5.0}, 
                                                          {"Call_35_Vehicle_7", 15.0}, 
                                                          {"Call_80_Vehicle_20", 15.0}, 
                                                          {"Call_130_Vehicle_40", 15.0}, 
                                                          {"Call_300_Vehicle_90", 15.0}};
    
    // Then run each instance in a multithreaded fashion
    std::vector<AlgorithmInformation> outputs;
    std::mutex outputsMutex;
    std::vector<std::thread> threads;
    for (int i = 2; i < instances.size(); i++) {
        std::string instance = instances[i].first;
        double time = instances[i].second;

        // Initialize a random number generator
        std::default_random_engine rng{std::random_device {}()};

        // Create a adaptive neighbourhood operator
        Operator* adaptiveOperator = new AdaptiveOperator({
            new SimilarGreedyInsert(),
            new SimilarRegretInsert(),
            new CostlyGreedyInsert(),
            new CostlyRegretInsert(),
            new RandomGreedyInsert(),
            new RandomRegretInsert(),
        });

        // Start a thread running the algorithm
        threads.emplace_back([i, &outputsMutex, &outputs, adaptiveOperator, instance, experiments, time, &rng](){
            // Get results
            AlgorithmInformation output = InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, instance, experiments, time, rng, "");

            // Sleep for ordering and push into vector
            std::this_thread::sleep_for(std::chrono::milliseconds(i*50));
            outputsMutex.lock();
            outputs.push_back(output);
            outputsMutex.unlock();
        });
    }

    // Run first two instances in main thread
    for (int i = 0; i < 2; i++) {
        std::string instance = instances[i].first;
        double time = instances[i].second;

        // Initialize a random number generator
        std::default_random_engine rng{std::random_device {}()};

        // Create a adaptive neighbourhood operator
        Operator* adaptiveOperator = new AdaptiveOperator({
            new SimilarGreedyInsert(),
            new SimilarRegretInsert(),
            new CostlyGreedyInsert(),
            new CostlyRegretInsert(),
            new RandomGreedyInsert(),
            new RandomRegretInsert(),
        });

        // Get results and push into vector
        AlgorithmInformation output = InstanceRunner::finalAdaptiveMetaheuristic(adaptiveOperator, instance, experiments, time, rng, "");
        outputsMutex.lock();
        outputs.push_back(output);
        outputsMutex.unlock();
    }
    


    // Wait for all threads to finish
    for (std::thread& thread : threads) {
        thread.join();
    }

    // And output information
    Debugger::outputToFile("results_final.txt");
    for (AlgorithmInformation& information : outputs) {
        for (EpisodeInformation& episode : information.episodes) {
            Debugger::printSolution(&episode.solution);
            std::cout << "Cost: " << std::to_string(episode.greedyCost);
            
            std::cout << " Actual: " << std::to_string(episode.actualCost) << ", found after iteration " << std::to_string(episode.iterfound) << " (" << Debugger::formatDouble(episode.timefound, 2) << " seconds)" << std::endl;
            std::cout << "Experiment ran for " << std::to_string(episode.totalIterations) << " iterations." << std::endl;
        }

        Debugger::printResults(information.instance, information.algorithm, information.averageObjective, information.bestSolution.getCost(), information.improvement, information.averageTime, &(information.bestSolution));
    }

    return 0;
}