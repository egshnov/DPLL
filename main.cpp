#include <iostream>

#include "lib/CNF.h"
#include "lib/DPLL.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "You have given " << argc - 1 << " arguments. You must pass ONE .cnf file that satisfies DIMACS."
                  << std::endl;
        return 1;
    }

    solver::CNF cnf;

    try {
        cnf.Parse(argv[1]);
    }
    catch (const std::string &error_message) {
        std::cerr << error_message << std::endl;
        return -1;
    }

    solver::DPLL(cnf);
    if (cnf.IsUnsat()) {
        std::cout << "UNSAT" << std::endl;
    } else {
        std::cout << cnf.ToString() << std::endl;
    }
    return 0;
}
