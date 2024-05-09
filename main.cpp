#include <iostream>
#include <fstream>
#include "lib/CNF.h"
#include "lib/DPLL.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "You have given " << argc << ". You must pass ONE .cnf file that satisfies DIMACS" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "couldn't open " << argv[1] << std::endl;
        return 1;
    }

    solver::CNF cnf;
    try {
        cnf.Init(input);
    }
    catch (...) {
        std::cerr << "Given file doesn't satisfy DIMACS format" << std::endl;
        return 1;
    }
    bool solve = solver::DPLL(cnf);
    if (solve) {
        std::cout << cnf.CnfToString();
    } else {
        std::cout << "UNSAT";
    }
    return 0;
}
