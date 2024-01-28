#include <vector>

#include "problem.h"

class Solution {
    public:
    /**
     * @brief Constructs the intial solution for the given problem.
     * The initial solution is the baseline solution, where none of the 
     * given calls are answered by "our" vehicles. All are "outsourced".
     * 
     * @param problem Problem to solve
     * @return Initial solution for the given problem
     */
    static Solution initialSolution(Problem* problem);

    private:
    // Only allow static instance creation
    Solution();

    Problem* problem;
    std::vector<int> representation;
};