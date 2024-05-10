#include "DPLL.h"
#include "CNF.h"
#include <iostream> //TODO: rm

namespace solver {

    namespace {
        bool real_solver(CNF &cnf, int branch) {
            cnf.UnitPropagation();

            if (cnf.IsUnsat()) {
                std::cout << cnf.ToString() << std::endl;// только UnitPropagation генерирует пустые
                return false;
            }

            cnf.PureLiterals();
            if (cnf.IsInterpretation()) return true; //TODO: optimize

            if (std::abs(branch) + 1 > cnf.GetVariablesNum()) {
                return false;
            }

            CNF tmp = cnf;
            tmp.AddUnitClauseFront(std::abs(branch) + 1);
            std::cout << "branch " << std::abs(branch) + 1 << std::endl;
            bool solved = real_solver(tmp, std::abs(branch) + 1);

            if (solved) {
                cnf = tmp;
                //std::cout << "SOLVED" << std::endl;
                return true;
            }

            tmp = cnf;
            tmp.AddUnitClauseFront(-(std::abs(branch) + 1));
            std::cout << "branch " << -(std::abs(branch) + 1) << std::endl;
            solved = real_solver(tmp, std::abs(branch) + 1);

            if (solved) {
                std::cout << "SOLVED" << std::endl;
                cnf = tmp;
            }
            return solved;
        }
    }

    bool DPLL(CNF &cnf) {
        bool solved = real_solver(cnf, 0);
        return solved;
    }
}
