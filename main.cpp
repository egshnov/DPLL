#include <iostream>
#include <chrono>

#include "CNF.h"
#include "DPLL.h"

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

    bool solved = solver::DPLL(cnf);

    if (!solved) {
        std::cout << "s UNSATISFIABLE" << std::endl;
    } else {
        std::cout << "s SATISFIABLE\nv ";
        for (int i = 0; i < cnf.GetVariablesNum(); i++) {
            int sign = cnf.GetModel()[i];
            if (sign == 0) {
                std::cout << i + 1 << " ";
            } else {
                std::cout << (i + 1) * sign << " ";
            }
        }
        std::cout << "0" << std::endl;
    }
    return 0;
}
