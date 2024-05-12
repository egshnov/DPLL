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

    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::steady_clock::now();
    bool solved = solver::DPLL(cnf);
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);


    if (!solved) {
        std::cout << "s UNSATISFIABLE" << std::endl;
    } else {
        std::cout << "s SATISFIABLE\nv ";
        for (int i = 1; i <= cnf.GetVariablesNum(); i++) {
            int p;
            try {
                p = cnf.GetAssignment().at(i);
            }
            catch (...) {
                p = i;
            }
            std::cout << p << " ";
        }
        std::cout << "0" << std::endl;
    }
    //std::cout << "elapsed time: " << time_span.count() << std::endl;
    return 0;
}
