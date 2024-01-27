#include <string>

class Parser {
    public:

    static std::string parseProblem(std::string path);

    private:

    // This is a static class, prevent class creation
    Parser();
};